<?php
	class UserResource extends UserController{
		protected function get(){		
			$user = $this->getResourceObject("ManagedAccess","id",$this->viewFromArguments("user"));
			$payload = $this->assemblePayload($user);
			ResponseHandler::retrieved($payload,$user->getURI(),$user->getModified(),$user->getETag());
		}
		
		protected function put(){
			$user = $this->getResourceObject("ManagedAccess","id",$this->viewFromArguments("user"));
			$normalisedUsername = strtolower($this->getFromContent("username"));
			$sqlGateway = new SQLGateway();
			
			//Set new user values
			$authentication = $this->getFromContent("authentication");
			$authentication = $authentication?md5($authentication):$user->getAuthentication();
			
			//Check whether username already exists
			$user->setUsername($normalisedUsername);
			if($user->isDirty('username')){
				$api = $this->getResourceObject("API","id",$this->viewFromArguments("api"));
				$searcher = new Searcher();
				$searcher->addCriterion("username",Criterion::EQUAL,$normalisedUsername);
				$searcher->addCriterion("api",Criterion::EQUAL,$api->getId());
				$searcher->addCriterion("id",Criterion::NOT_EQUAL,$user->getId());
				if($sqlGateway->findUnique("ManagedAccess",$searcher)){
					ResponseHandler::conflict("A user identified by '{$this->getFromContent('username')}' already exists.");
				}
			}
			
			//Update User Options
			$user->insertToOptions("role",$this->getFromContent("role"));
			foreach($this->getFromContent("options",array()) as $option => $value){
				$user->insertToOptions($option,$value);
			}
			
			if($user->isDirty("options")){
				$api = $this->getResourceObject("API","id",$this->viewFromArguments("api"));
				$request = new WadapiRequest(parse_url($api->getRoot(),PHP_URL_HOST));
				if($api->isSecure()){
					$request->setTransport("ssl");
					$request->setPort(443);
				}
			
				$request->setEndpoint($user->getAccessEndpoint());
				$request->authorise($api->getAccessKey(),$api->getAccessSecret());
				$request->setContentType("application/json");
				$request->assureConsistency();
				$payload = json_encode($user->getOptions());
				$request->setBody($payload);
				$response = $request->put();
				$body = json_decode($response->getBody(),true);
			}
			
			//Update user password if necessary
			$user->setAuthentication($authentication);
			if($user->isDirty("authentication")){
				$api = $this->getResourceObject("API","id",$this->viewFromArguments("api"));
				
				//Delete Old Access Tokens
				$request = new WadapiRequest(parse_url($api->getRoot(),PHP_URL_HOST));
				if($api->isSecure()){
					$request->setTransport("ssl");
					$request->setPort(443);
				}
				
				$request->setEndpoint($user->getAccessEndpoint());
				$request->authorise($api->getAccessKey(),$api->getAccessSecret());
				$request->setContentType("application/json");
				$request->assureConsistency();
				$response = $request->delete();
				
				//Create Fresh Access Tokens
				$request->setEndpoint(parse_url($api->getRoot(),PHP_URL_PATH)."/access");
				$request->authorise($api->getAccessKey(),$api->getAccessSecret());
				$request->setContentType("application/json");
				$payload = json_encode($user->getOptions());
				
				$request->setBody($payload);
				$response = $request->post();
				$body = json_decode($response->getBody(),true);
				
				if($response->getStatusCode() == 400){
					ResponseHandler::bad(str_replace("arguments","directives",$body["message"]));
				}else if($response->getStatusCode() != 201){
					ResponseHandler::error("The server is misconfigured. Please contact a system administrator");
				}
			
				$responseBody = json_decode($response->getBody(),true);
				foreach($responseBody as $option => $value){
					if(!in_array($option,array("self","tokens","active-token","role"))){
						$user->insertToOptions($option,$value);
					}
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
				
				$user->setAccessKey($this->encrypt($accessKey,$this->getFromContent("authentication")));
				$user->setAccessSecret($this->encrypt($accessSecret,$this->getFromContent("authentication")));
				$user->setRefreshSecret($this->encrypt($refreshSecret,$this->getFromContent("authentication")));
				$user->setAccessEndpoint($accessEndpoint);
				$user->setExpires(time()+$lifetime);
			}
			
			//If the user's username or password have changed, Update APIToken for User to Access its credentials
			if($user->isDirty("username") || $user->isDirty("authentication")){
				$searcher = new Searcher();
				$searcher->addCriterion("accessKey",Criterion::EQUAL,md5(Photographer::getSnapshot($user,"username")));
				$apiToken = $sqlGateway->findUnique("APIToken",$searcher);
				
				$apiToken->setAccessKey(md5($user->getUsername()));
				$apiToken->setAccessSecret($user->getAuthentication());
				$apiToken->setRefreshSecret($user->getAuthentication());
				$sqlGateway->save($apiToken);
			}
			
			$sqlGateway->save($user);
			
			$payload = $this->assemblePayload($user);
			ResponseHandler::modified($payload,$user->getURI());
		}
		
		protected function delete(){
			$user = $this->getResourceObject("ManagedAccess","id",$this->viewFromArguments("user"));
			$api = $this->getResourceObject("API","id",$this->viewFromArguments("api"));
			
			$request = new WadapiRequest(parse_url($api->getRoot(),PHP_URL_HOST));
			if($api->isSecure()){
				$request->setTransport("ssl");
				$request->setPort(443);
			}
			
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