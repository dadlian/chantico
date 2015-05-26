<?php
	abstract class ChanticoController extends PagedResourceController{
		public function execute(){
			$sqlGateway = new SQLGateway();
			$searcher = new Searcher();
			
			$searcher->addCriterion("title",Criterion::EQUAL,"blacklist");
			$blacklist = $sqlGateway->findUnique("IPList",$searcher);
			if($blacklist && in_array($_SERVER['REMOTE_ADDR'],$blacklist->getEntries())){
				ResponseHandler::forbidden("The originating IP address is not permitted to make requests.");
			}
			
			$searcher->clearCriteria();
			$searcher->addCriterion("title",Criterion::EQUAL,"whitelist");
			$whitelist = $sqlGateway->findUnique("IPList",$searcher);
			if($whitelist && $whitelist->getEntries() && !in_array($_SERVER['REMOTE_ADDR'],$whitelist->getEntries())){
				ResponseHandler::forbidden("The originating IP address is not permitted to make requests.");
			}
			
			parent::execute();
		}
		
		protected function getInvalidQueryParameters(){
		}
		
		protected function getRecordCount(){
		}
		
		protected function buildPageEntries($start,$count){
		}
	}
?>