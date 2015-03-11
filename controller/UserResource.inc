<?php
	class UserResource extends UserController{
		protected function get(){
			$user = $this->getResourceObject("ManagedAccess","id",$this->viewFromArguments("user"));
			$payload = $this->assemblePayload($user);
			ResponseHandler::retrieved($payload,$user->getURI(),$user->getModified(),$user->getETag());
		}
		
		protected function put(){
			$user = $this->getResourceObject("ManagedAccess","id",$this->viewFromArguments("user"));
		
			$sqlGateway = new SQLGateway();
			$searcher = new Searcher();
			$searcher->addCriterion("username",Criterion::EQUAL,$this->getFromContent("username"));
			$searcher->addCriterion("id",Criterion::NOT_EQUAL,$user->getId());
			if($sqlGateway->findUnique("ManagedAccess",$searcher)){
				ResponseHandler::conflict("A user identified by '{$this->getFromContent('username')}' already exists.");
			}
			
			//Update Access
			$request = new WadapiRequest(parse_url(SettingsManager::getSetting("options","apiuri"),PHP_URL_HOST));
			$request->setEndpoint($user->getAccessEndpoint());
			$request->authorise(SettingsManager::getSetting("options","key"),SettingsManager::getSetting("options","secret"));
			$request->setContentType("application/json");
			$request->assureConsistency();
			$payload = json_encode(array_merge(array("role"=>$this->getFromContent("role")),$this->getFromContent("options",array())));
			$request->setBody($payload);
			$response = $request->put();
			$body = json_decode($response->getBody(),true);
			
			if($response->getStatusCode() == 400){
				ResponseHandler::bad($body["message"]);
			}else if($response->getStatusCode() != 200){
				ResponseHandler::error("The server is misconfigured. Please contact a system administrator");
			}
			
			//Get corresponding access token
			$searcher->clearCriteria();
			$searcher->addCriterion("accessKey",Criterion::EQUAL,md5($user->getUsername()));
			$apiToken = $sqlGateway->findUnique("APIToken",$searcher);
			
			//Save user changes to Chantico
			$user->setUsername($this->getFromContent("username"));
			$user->setAuthentication(md5($this->getFromContent("authentication")));
			$sqlGateway->save($user);
			
			//Update APIToken for User to Access its tokens
			$apiToken->setAccessKey(md5($user->getUsername()));
			$apiToken->setAccessSecret($user->getAuthentication());
			$apiToken->setRefreshSecret($user->getAuthentication());
			$sqlGateway->save($apiToken);
			
			$payload = $this->assemblePayload($user);
			ResponseHandler::modified($payload,$user->getURI());
		}
		
		protected function delete(){
			$user = $this->getResourceObject("ManagedAccess","id",$this->viewFromArguments("user"));
			CryptKeeper::bury($user);
			ResponseHandler::deleted("User: /".RequestHandler::getRequestURI().", has been removed.");
		}
	
		protected function isConsistent($modifiedDate,$eTag){
			$user = $this->getResourceObject("ManagedAccess","id",$this->viewFromArguments("user"));
			return $modifiedDate == $user->getModified() && $eTag == $user->getETag();
		}
	}
?>