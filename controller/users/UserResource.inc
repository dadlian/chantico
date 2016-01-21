<?php
	class UserResource extends UserController{
		protected function get(){		
			$user = $this->getResourceObject("ManagedAccess","id",$this->viewFromArguments("user"));
			$payload = $this->assemblePayload($user);
			ResponseHandler::retrieved($payload,$user->getURI(),$user->getModified(),$user->getETag());
		}
		
		protected function put(){
			$user = $this->getResourceObject("ManagedAccess","id",$this->viewFromArguments("user"));
			$api = $this->getResourceObject("API","id",$this->viewFromArguments("api"));
			$normalisedUsername = strtolower($this->getFromContent("username"));
			
			$sqlGateway = new SQLGateway();
			$searcher = new Searcher();
			$searcher->addCriterion("username",Criterion::EQUAL,$normalisedUsername);
			$searcher->addCriterion("id",Criterion::NOT_EQUAL,$user->getId());
			if($sqlGateway->findUnique("ManagedAccess",$searcher)){
				ResponseHandler::conflict("A user identified by '{$this->getFromContent('username')}' already exists.");
			}
			
			//Get user's APIToken
			$searcher->clearCriteria();
			$searcher->addCriterion("accessKey",Criterion::EQUAL,md5($user->getUsername()));
			$apiToken = $sqlGateway->findUnique("APIToken",$searcher);
			
			//Save user changes to Chantico
			$authentication = $this->getFromContent("authentication");
			$user->setUsername($normalisedUsername);
			if($authentication){
				//Delete Old Access Tokens
				$request = new WadapiRequest(parse_url($api->getRoot(),PHP_URL_HOST));
				$request->setEndpoint($user->getAccessEndpoint());
				$request->authorise($api->getAccessKey(),$api->getAccessSecret());
				$request->setContentType("application/json");
				$request->assureConsistency();
				$response = $request->delete();
				
				//Create Fresh Access Tokens
				$request->setEndpoint(parse_url($api->getRoot(),PHP_URL_PATH)."/access");
				$request->authorise($api->getAccessKey(),$api->getAccessSecret());
				$request->setContentType("application/json");
				$payload = json_encode(array_merge(array("role"=>$this->getFromContent("role")),$this->getFromContent("options",array())));
				$request->setBody($payload);
				$response = $request->post();
				$body = json_decode($response->getBody(),true);
				
				if($response->getStatusCode() == 400){
					ResponseHandler::bad(str_replace("arguments","directives",$body["message"]));
				}else if($response->getStatusCode() != 201){
					ResponseHandler::error("The server is misconfigured. Please contact a system administrator");
				}
				
				$accessEndpoint = parse_url($body["self"],PHP_URL_PATH);
				$tokensEndpoint = parse_url($body["tokens"],PHP_URL_PATH);
				$accessKey = $body["active-token"]["key"];
				$refreshSecret = $body["active-token"]["refresh"];
				
				//Refresh Access Token
				$request->setEndpoint($tokensEndpoint);
				$request->authorise($accessKey,$refreshSecret);
				$request->setBody(json_encode(array("lifetime"=>SettingsManager::getSetting("options","lifetime"))));
				$response = $request->post();
				
				if($response->getStatusCode() != 201){
					ResponseHandler::error("The server is misconfigured. Please contact a system administrator");
				}
				
				$body = json_decode($response->getBody(),true);
				$accessKey = $body["key"];
				$accessSecret = $body["secret"];
				$refreshSecret = $body["refresh"];
				$lifetime = $body["lifetime"];
				
				$user->setAuthentication(md5($authentication));
				$user->setAccessKey($this->encrypt($accessKey,$authentication));
				$user->setAccessSecret($this->encrypt($accessSecret,$authentication));
				$user->setRefreshSecret($this->encrypt($refreshSecret,$authentication));
				$user->setAccessEndpoint($accessEndpoint);
				$user->setExpires(time()+$lifetime);
			}else{
				//Update Access
				$request = new WadapiRequest(parse_url($api->getRoot(),PHP_URL_HOST));
				$request->setEndpoint($user->getAccessEndpoint());
				$request->authorise($api->getAccessKey(),$api->getAccessSecret());
				$request->setContentType("application/json");
				$request->assureConsistency();
				$payload = json_encode(array_merge(array("role"=>$this->getFromContent("role")),$this->getFromContent("options",array())));
				$request->setBody($payload);
				$response = $request->put();
				$body = json_decode($response->getBody(),true);
			}
			
			$user->setOptions($this->getFromContent("options",array()));
			$sqlGateway->save($user);
			
			//Update APIToken for User to Access its
			$apiToken->setAccessKey(md5($user->getUsername()));
			$apiToken->setAccessSecret($user->getAuthentication());
			$apiToken->setRefreshSecret($user->getAuthentication());
			$sqlGateway->save($apiToken);
			
			$payload = $this->assemblePayload($user);
			ResponseHandler::modified($payload,$user->getURI());
		}
		
		protected function delete(){
			$user = $this->getResourceObject("ManagedAccess","id",$this->viewFromArguments("user"));
			$api = $this->getResourceObject("API","id",$this->viewFromArguments("api"));
			
			$request = new WadapiRequest(parse_url($api->getRoot(),PHP_URL_HOST));
			$request->setEndpoint($user->getAccessEndpoint());
			$request->authorise($api->getAccessKey(),$api->getAccessSecret());
			$request->setContentType("application/json");
			$request->assureConsistency();
			$response = $request->delete();
			
			if($response->getStatusCode() == 200){
				CryptKeeper::bury($user);
				
				//Delete APIToken associated with user
				$sqlGateway = new SQLGateway();
				$searcher = new Searcher();
				$searcher->addCriterion("accessKey",Criterion::EQUAL,md5($user->getUsername()));
				$apiToken = $sqlGateway->findUnique("APIToken",$searcher);
				$sqlGateway->delete($apiToken);
				
				//Delete Renewals associated with user
				$sqlGateway->delete($user->getRenewals());
				
				ResponseHandler::deleted("User: /".RequestHandler::getRequestURI().", has been removed.");
			}else{
				ResponseHandler::error("User: /".RequestHandler::getRequestURI().", cannot be removed.");
			}
		}
	
		protected function isConsistent($modifiedDate,$eTag){
			$user = $this->getResourceObject("ManagedAccess","id",$this->viewFromArguments("user"));
			return $modifiedDate == $user->getModified() && $eTag == $user->getETag();
		}
	}
?>