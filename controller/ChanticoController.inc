<?php
	abstract class ChanticoController extends PagedResourceController{
		public function execute(){
			$api = $this->getResourceObject("API","id",$this->viewFromArguments("api"));
			
			$blacklist = $api->getBlacklist();
			if($blacklist && in_array($_SERVER['REMOTE_ADDR'],$blacklist->getEntries())){
				ResponseHandler::forbidden("The originating IP address is not permitted to make requests.");
			}
			
			$whitelist = $api->getWhitelist();
			if($whitelist && $whitelist->getEntries() && !in_array($_SERVER['REMOTE_ADDR'],$whitelist->getEntries())){
				ResponseHandler::forbidden("The originating IP address is not permitted to make requests.");
			}
			
			parent::execute();
		}
		
		protected function getResourceObject($class,$field,$value){
			if($class == "ManagedAccess"){
				$api = $this->getResourceObject("API","id",$this->viewFromArguments("api"));
				
				$sqlGateway = new SQLGateway();
				$searcher = new Searcher();
				$searcher->addCriterion($field,Criterion::EQUAL,$value);
				$searcher->addCriterion("api",Criterion::EQUAL,$api->getId());
				
				$user = $sqlGateway->findUnique($class,$searcher);
				if(!$user && !CryptKeeper::exhume($value,$field)){
					ResponseHandler::missing("There is presently no resource with the given URI.");
				}
			}
			
			return parent::getResourceObject($class,$field,$value);
		}
		
		protected function getInvalidQueryParameters(){
		}
		
		protected function getRecordCount(){
		}
		
		protected function buildPageEntries($start,$count){
		}
	}
?>