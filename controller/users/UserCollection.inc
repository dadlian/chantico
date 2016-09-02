<?php
	class UserCollection extends UserController{
		protected function post(){
			$api = $this->getResourceObject("API","id",$this->viewFromArguments("api"));
			$normalisedUsername = strtolower($this->getFromContent("username"));
			
			$sqlGateway = new SQLGateway();
			$searcher = new Searcher();
			$searcher->addCriterion("username",Criterion::EQUAL,$normalisedUsername);
			$searcher->addCriterion("api",Criterion::EQUAL,$api->getId());
			if($sqlGateway->findUnique("ManagedAccess",$searcher)){
				ResponseHandler::conflict("A user identified by '$normalisedUsername' already exists.");
			}
			
			//Create New Access
			$request = new WadapiRequest(parse_url($api->getRoot(),PHP_URL_HOST));
			$request->setTransport("ssl");
			$request->setPort(443);
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
			
			$options = array();
			if($response->getStatusCode() == 201){
				$responseBody = json_decode($response->getBody(),true);
				foreach($responseBody as $option => $value){
					if(!in_array($option,array("self","tokens","active-token","role"))){
						$options[$option] = $value;
					}
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
			
			//Save User to Chantico
			$user = new ManagedAccess();
			$user->setUsername($normalisedUsername);
			$user->setAuthentication(md5($this->getFromContent("authentication")));
			$user->setAccessKey($this->encrypt($accessKey,$this->getFromContent("authentication")));
			$user->setAccessSecret($this->encrypt($accessSecret,$this->getFromContent("authentication")));
			$user->setRefreshSecret($this->encrypt($refreshSecret,$this->getFromContent("authentication")));
			$user->setAccessEndpoint($accessEndpoint,$this->getFromContent("authentication"));
			$user->setExpires(time()+$lifetime);
			
			$optionsArray = $this->getFromContent("options",$options);
			if(!array_key_exists("role",$optionsArray)){
				$optionsArray["role"] = $this->getFromContent("role");
			}
			$user->setOptions($optionsArray);
			
			$user->setApi($api->getId());
			$sqlGateway->save($user);
			
			//Create APIToken for User to Access its tokens
			$sqlGateway->save(new APIToken("user",0,md5($user->getUsername()),$user->getAuthentication(),$user->getAuthentication()));
			
			$payload = $this->assemblePayload($user);
			ResponseHandler::created($payload,$user->getURI());
		}
		
		protected function isConsistent($modifiedDate,$eTag){
			return true;
		}
	}
?>