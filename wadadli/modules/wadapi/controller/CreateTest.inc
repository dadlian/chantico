<?php
	class CreateTest extends ResourceController{
		public function post(){
			$sqlGateway = new SQLGateway();
			$arguments = RequestHandler::getContent();
			
			//Create Test Organisation
			$testName = (array_key_exists("name",$arguments)?$arguments["name"]:"Test Run");
			$nameParts = explode(" ",$testName);
			$slug = strtolower($nameParts[sizeof($nameParts)-1]);
			$organisation = new Organisation($slug,$arguments["name"]);
			$organisation->reconcileSlug();
			
			//Create Root Access Token
			$testToken = new APIToken("root");
			$accessTokens = $testToken->refresh(600);
			
			//Add Token to Organisation
			$organisation->insertToTokens($testToken->getId(),$testToken);
			$sqlGateway->save($organisation);
						
			//Send access information to client
			$access = array("key"=>$accessTokens["key"],
					"secret"=>$accessTokens["secret"],
					"organisation"=>"{$organisation->getURI()}");
			ResponseHandler::created($access,"{$testToken->getURI()}/tokens/active");
		}
		
		protected function isInvalid(){
			return array();
		}
		
		protected function isConsistent($modifiedDate,$eTag){
			return true;
		}
		
		protected function assemblePayload($object){
			return "";
		}
	}
?>