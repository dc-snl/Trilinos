#ifndef MUELU_COALESCEDROPFACTORY_DEF_HPP
#define MUELU_COALESCEDROPFACTORY_DEF_HPP

#include <Xpetra_Operator.hpp>
#include <Xpetra_MultiVector.hpp>
#include <Xpetra_VectorFactory.hpp>
#include <Xpetra_ImportFactory.hpp>
#include <Xpetra_MapFactory.hpp>
#include <Xpetra_CrsGraph.hpp>
#include <Xpetra_CrsGraphFactory.hpp>
#include <Xpetra_StridedMap.hpp>

#include "MueLu_CoalesceDropFactory_decl.hpp"

#include "MueLu_Level.hpp"
#include "MueLu_Graph.hpp"
#include "MueLu_PreDropFunctionBaseClass.hpp"
#include "MueLu_PreDropFunctionConstVal.hpp"
#include "MueLu_Monitor.hpp"
#include "MueLu_AmalgamationInfo.hpp"
#include "MueLu_SubBlockUnAmalgamationFactory.hpp"

namespace MueLu {



template <class Scalar,class LocalOrdinal, class GlobalOrdinal, class Node, class LocalMatOps>
CoalesceDropFactory<Scalar, LocalOrdinal, GlobalOrdinal, Node, LocalMatOps>::CoalesceDropFactory(RCP<const FactoryBase> AFact)
: AFact_(AFact)
  {
    predrop_ = Teuchos::null;
    UnAmalgamationFact_ = rcp(new SubBlockUnAmalgamationFactory(AFact));
  }

template <class Scalar,class LocalOrdinal, class GlobalOrdinal, class Node, class LocalMatOps>
void CoalesceDropFactory<Scalar, LocalOrdinal, GlobalOrdinal, Node, LocalMatOps>::DeclareInput(Level &currentLevel) const {
  currentLevel.DeclareInput("A", AFact_.get(), this);
  currentLevel.DeclareInput("UnAmalgamationInfo", UnAmalgamationFact_.get(), this);
}

template <class Scalar,class LocalOrdinal, class GlobalOrdinal, class Node, class LocalMatOps>
void CoalesceDropFactory<Scalar, LocalOrdinal, GlobalOrdinal, Node, LocalMatOps>::SetPreDropFunction(const RCP<MueLu::PreDropFunctionBaseClass<Scalar,LocalOrdinal,GlobalOrdinal,Node,LocalMatOps> > &predrop) { predrop_ = predrop; }

template <class Scalar,class LocalOrdinal, class GlobalOrdinal, class Node, class LocalMatOps>
void CoalesceDropFactory<Scalar, LocalOrdinal, GlobalOrdinal, Node, LocalMatOps>::Build(Level &currentLevel) const {
  FactoryMonitor m(*this, "CoalesceDropFactory2", currentLevel);
  if(predrop_ != Teuchos::null) {
    GetOStream(Parameters0, 0) << predrop_->description();
  }

  RCP<Teuchos::FancyOStream> out = Teuchos::fancyOStream(Teuchos::rcpFromRef(std::cout));

  RCP<Operator> A = currentLevel.Get< RCP<Operator> >("A", AFact_.get());


  LocalOrdinal blockdim = 1;         // block dim for fixed size blocks
  GlobalOrdinal offset = 0;          // global offset of dof gids

  // 1) check for blocking/striding information
  if(A->IsView("stridedMaps") &&
      Teuchos::rcp_dynamic_cast<const StridedMap>(A->getRowMap("stridedMaps")) != Teuchos::null) {
    Xpetra::viewLabel_t oldView = A->SwitchToView("stridedMaps"); // note: "stridedMaps are always non-overlapping (correspond to range and domain maps!)
    RCP<const StridedMap> strMap = Teuchos::rcp_dynamic_cast<const StridedMap>(A->getRowMap());
    TEUCHOS_TEST_FOR_EXCEPTION(strMap == Teuchos::null,Exceptions::BadCast,"MueLu::CoalesceFactory::Build: cast to strided row map failed.");
    blockdim = strMap->getFixedBlockSize(); // TODO shorten code
    offset   = strMap->getOffset();
    oldView = A->SwitchToView(oldView);
    GetOStream(Debug, 0) << "CoalesceDropFactory::Build():" << " found blockdim=" << blockdim << " from strided maps. offset=" << offset << std::endl;
  } else GetOStream(Debug, 0) << "CoalesceDropFactory::Build(): no striding information available. Use blockdim=1 with offset=0" << std::endl;
  
  // 2) build (un)amalgamation information 
  //    prepare generation of nodeRowMap (of amalgamated matrix)
  // TODO: special handling for blockdim=1
  RCP<AmalgamationInfo> amalInfo = currentLevel.Get< RCP<AmalgamationInfo> >("UnAmalgamationInfo", UnAmalgamationFact_.get());
  RCP<std::map<GlobalOrdinal,std::vector<GlobalOrdinal> > > nodegid2dofgids = amalInfo->GetGlobalAmalgamationParams();
  RCP<std::vector<GlobalOrdinal> > gNodeIds = amalInfo->GetNodeGIDVector();
  GlobalOrdinal cnt_amalRows = amalInfo->GetNumberOfNodes();
  
  // inter processor communication: sum up number of block ids
  GlobalOrdinal num_blockids = 0;
  Teuchos::reduceAll<int,GlobalOrdinal>(*(A->getRowMap()->getComm()),Teuchos::REDUCE_SUM, cnt_amalRows, Teuchos::ptr(&num_blockids) );
  GetOStream(Debug, 0) << "CoalesceDropFactory::SetupAmalgamationData()" << " # of amalgamated blocks=" << num_blockids << std::endl;

  // 3) generate row map for amalgamated matrix (graph of A)
  //    with same distribution over all procs as row map of A
  Teuchos::ArrayRCP<GlobalOrdinal> arr_gNodeIds = Teuchos::arcp( gNodeIds );
  Teuchos::RCP<Map> nodeMap = MapFactory::Build(A->getRowMap()->lib(), num_blockids, arr_gNodeIds(), A->getRowMap()->getIndexBase(), A->getRowMap()->getComm());
  GetOStream(Debug, 0) << "CoalesceDropFactory: nodeMap " << nodeMap->getNodeNumElements() << "/" << nodeMap->getGlobalNumElements() << " elements" << std::endl;

  // 4) create graph of amalgamated matrix
  RCP<CrsGraph> crsGraph = CrsGraphFactory::Build(nodeMap, 10, Xpetra::DynamicProfile);

  // 5) do amalgamation. generate graph of amalgamated matrix
  for(LocalOrdinal row=0; row<Teuchos::as<LocalOrdinal>(A->getRowMap()->getNodeNumElements()); row++) {
    // get global DOF id
    GlobalOrdinal grid = A->getRowMap()->getGlobalElement(row);

    // translate grid to nodeid
    GlobalOrdinal nodeId = SubBlockUnAmalgamationFactory::DOFGid2NodeId(grid, A, blockdim, offset);

    size_t nnz = A->getNumEntriesInLocalRow(row);
    Teuchos::ArrayView<const LocalOrdinal> indices;
    Teuchos::ArrayView<const Scalar> vals;
    A->getLocalRowView(row, indices, vals);
    TEUCHOS_TEST_FOR_EXCEPTION(Teuchos::as<size_t>(indices.size()) != nnz, Exceptions::RuntimeError, "MueLu::CoalesceFactory::Amalgamate: number of nonzeros not equal to number of indices? Error.");

    RCP<std::vector<GlobalOrdinal> > cnodeIds = Teuchos::rcp(new std::vector<GlobalOrdinal>);  // global column block ids
    LocalOrdinal realnnz = 0;
    for(LocalOrdinal col=0; col<Teuchos::as<LocalOrdinal>(nnz); col++) {
      TEUCHOS_TEST_FOR_EXCEPTION(A->getColMap()->isNodeLocalElement(indices[col])==false,Exceptions::RuntimeError, "MueLu::CoalesceFactory::Amalgamate: Problem with columns. Error.");
      GlobalOrdinal gcid = A->getColMap()->getGlobalElement(indices[col]); // global column id

      //if(vals[col]!=0.0) {
      if((predrop_ == Teuchos::null && vals[col]!=0.0) ||
         (predrop_ != Teuchos::null && predrop_->Drop(row,grid, col,indices[col],gcid,indices,vals) == false)) {
        GlobalOrdinal cnodeId = SubBlockUnAmalgamationFactory::DOFGid2NodeId(gcid, A, blockdim, offset);
        cnodeIds->push_back(cnodeId);
        realnnz++; // increment number of nnz in matrix row
      }
    }

    Teuchos::ArrayRCP<GlobalOrdinal> arr_cnodeIds = Teuchos::arcp( cnodeIds );


    TEUCHOS_TEST_FOR_EXCEPTION(crsGraph->getRowMap()->isNodeGlobalElement(nodeId)==false,Exceptions::RuntimeError, "MueLu::CoalesceFactory::Amalgamate: global row id does not belong to current proc. Error.");
    crsGraph->insertGlobalIndices(nodeId, arr_cnodeIds());
  }
  // fill matrix graph
  crsGraph->fillComplete(nodeMap,nodeMap);

  // 6) create MueLu Graph object
  RCP<Graph> graph = rcp(new Graph(crsGraph, "amalgamated graph of A"));

  // store information in Graph object for unamalgamation of vectors
  // TODO remove this
  //graph->SetAmalgamationParams(nodegid2dofgids);

  // store (un)amalgamation information on current level
  // i.e. redeclare UnAmalgamationInfo generated by CoalesceDropFactory
  RCP<AmalgamationInfo> amalgamationData = rcp(new AmalgamationInfo());
  amalgamationData->SetAmalgamationParams(nodegid2dofgids);
  currentLevel.Set("UnAmalgamationInfo", amalgamationData, this);

  // 7) store results in Level
  currentLevel.Set("DofsPerNode", blockdim, this);
  currentLevel.Set("Graph", graph, this);
}

/*template <class Scalar,class LocalOrdinal, class GlobalOrdinal, class Node, class LocalMatOps>
GlobalOrdinal CoalesceDropFactory<Scalar, LocalOrdinal, GlobalOrdinal, Node, LocalMatOps>::DOFGid2NodeId(GlobalOrdinal gid, const RCP<Operator>& A, LocalOrdinal blockSize, const GlobalOrdinal offset) const {
  //GetOStream(Runtime0, 0) << "fixed block size..." << std::endl;
  GlobalOrdinal globalblockid = ((GlobalOrdinal) gid - offset) / blockSize;
  return globalblockid;
}*/

} //namespace MueLu

#endif // MUELU_COALESCEDROPFACTORY_DEF_HPP
