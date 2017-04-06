<?php
	class APIResource extends APIController{
		protected function get(){
			$api = $this->getResourceObject("API","id",$this->viewFromArguments("api"));
			$payload = $this->assemblePayload($api);
			
			ResponseHandler::retrieved($payload,$api->getURI(),$api->getModified(),$api->getETag());
		}
		
		protected function put(){
			$api = $this->getResourceObject("API","id",$this->viewFromArguments("api"));
			
			$sqlGateway = new SQLGateway();
			$searcher = new Searcher();
			
			$root = $this->getFromContent("root");
			$searcher->addCriterion("root",Criterion::EQUAL,$this->getFromContent("root"));
			if($sqlGateway->findUnique("API",$searcher)){
				ResponseHandler::conflict("API {$this->getFromContent("root")} has already been registered with Chantico.");
			}
			
			//Update API
			$accessKey = $this->getFromContent("access-key");
			$accessSecret = $this->getFromContent("access-secret");
			$secure = $this->getFromContent("secure",true);
			
			$api->setAccessKey($accessKey);
			$api->setAccessSecret($accessSecret);
			$api->setSecure($secure);
			$sqlGateway->save($api);
			
			$payload = $this->assemblePayload($api);
			ResponseHandler::modified($payload,$api->getURI(),$api->getModified(),$api->getETag());
		}
		
		protected function delete(){
			$api = $this->getResourceObject("API","id",$this->viewFromArguments("api"));
			
			//Don't delete an api with registered users
			$sqlGateway = new SQLGateway();
			$searcher = new Searcher();
			$searcher->addCriterion("api",Criterion::EQUAL,$api->getId());
			if($sqlGateway->findUnique("ManagedAccess",$searcher)){
				ResponseHandler::conflict("An API with managed users cannot be deleted.");
			}
			
			CryptKeeper::bury($api);
			ResponseHandler::deleted("API: {$api->getRoot()}, has been deregistered.");
		}
		
		protected function isConsistent($modifiedDate,$eTag){
			$api = $this->getResourceObject("API","id",$this->viewFromArguments("api"));
			return $modifiedDate == $api->getModified() && $eTag == $api->getETag();
		}
	}
?>