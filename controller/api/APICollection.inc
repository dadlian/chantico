<?php
	class APICollection extends APIController{
		protected function post(){
			$sqlGateway = new SQLGateway();
			$searcher = new Searcher();
			
			$root = $this->getFromContent("root");
			$searcher->addCriterion("root",Criterion::EQUAL,$this->getFromContent("root"));
			if($sqlGateway->findUnique("API",$searcher)){
				ResponseHandler::conflict("API {$this->getFromContent("root")} has already been registered with Chantico.");
			}
			
			//Create API Entry
			$accessKey = $this->getFromContent("access-key");
			$accessSecret = $this->getFromContent("access-secret");
			$secure = $this->getFromContent("secure",true);
			
			$api = new API($root,$accessKey,$accessSecret,new IPList(),new IPList(),$secure);
			$sqlGateway->save($api);
			
			$payload = $this->assemblePayload($api);
			ResponseHandler::created($payload,$api->getURI());
		}
		
		protected function isConsistent($modifiedDate,$eTag){
			return true;
		}
	}
?>