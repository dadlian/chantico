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
			if($api->isSecure()){
				$request->setTransport("ssl");
				$request->setPort(443);
			}

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
			$user->setAccessEndpoint($accessEndpoint);
			$user->setExpires(time()+$lifetime);

			$optionsArray = array();
			foreach($this->getFromContent("options",$options) as $option => $value){
				$optionsArray[$option] = $value?json_encode($value):$value;
			}

			if(!array_key_exists("role",$optionsArray)){
				$optionsArray["role"] = $this->getFromContent("role");
			}
			$user->setOptions($optionsArray);

			$user->setApi($api->getId());
			$sqlGateway->save($user);

			//Create APIToken for User to Access its tokens
			$sqlGateway->save(new APIToken("user",0,md5($user->getUsername()),$user->getAuthentication(),$user->getAuthentication()));

			//Notify user of new Account
			if(in_array($api->getId(),[14915827114381,14915100262884])){
				$this->notify($user,$options);
			}

			//Build Payload and Send Response
			$payload = array(
				"self"=>$user->getURI(),
				"username"=>$user->getUsername(),
				"role"=>$user->viewFromOptions("role"),
				"access"=>"{$api->getRoot()}{$user->getAccessEndpoint()}",
				"tokens"=>"{$user->getURI()}/tokens",
				"options"=>$options,
				"resets"=>"{$user->getURI()}/resets",
				"recoveries"=>"{$user->getURI()}/recoveries"
			);

			ResponseHandler::created($payload,$user->getURI());
		}

		protected function isConsistent($modifiedDate,$eTag){
			return true;
		}

		private function notify($user,$options){
			$template = "<html>
				<head>
					<title>New ACYM Account</title>
				</head>
				<body>
					<table>
						<tr>
							<td>
								<img style='float:left; width:25%;' src='https://registration.antiguacharteryachtmeeting.com/assets/email_logo.jpg' alt='ACYM logo' style='border:none;'>
							</td>
						</tr>
						<tr>
							<td>
								<h3 style='font-size:14px; font-family:Verdana;'>Dear {$options['first-name']},</h3>
								<p style='font-size:14px; font-family:Verdana;'>Your personal account for the Antigua Charter Yacht Meeting has successfully been created.</p>
							</td>
						</tr>
						<tr>
							<td>
								<div>Username:</div>
								<div><strong>{$user->getUsername()}</strong></div>
							</td>
						</tr>
						<tr>
							<td>
								<p style='font-size:14px; font-family:Verdana;'>
									You may now <a href='https://registration.antiguacharteryachtmeeting.com/#/profile'>submit a
									registration application</a> to attend any of our upcoming events.
								</p>
							</td>
						</tr>
						<tr>
							<td height='20px'>
							</td>
						</tr>
						<tr>
							<td>
								<div>Regards,</div>
								<div>Antigua Charter Yacht Meeting</div>
								<div>Tel.: +1 (268) 460-1059</div>
								<div><a href='http://antiguayachtshow.com'>http://antiguayachtshow.com</a></div>
							</td>
						</tr>
						<tr>
							<td><hr /></td>
						</tr>
						<tr>
							<td bgcolor='#1489BE' style='color:#FFFFFF;padding:10px;font-size:12px;text-align:center;'>
									&copy; ".date('Y')." Antigua Charter Yacht Meeting, Antigua, W.I.
							</td>
						</tr>
					</table>
				</body>
			<html>";

			$url = "https://api:key-eb68c3a3cb2e35aef236e49328b92ddb@api.mailgun.net/v3/antiguacharteryachtmeeting.com/messages";
			$payload = array(
				"to"=>"{$options['first-name']} {$options['last-name']} <{$options['email']}>",
				"from"=> "Antigua Charter Yacht Meeting <noreply@antiguacharteryachtmeeting.com>",
				"subject"=>"Your new ACYM Account has been created",
				"html"=> $template
			);

			$header = array(
				'Content-Type: application/x-www-form-urlencoded'
			);

			$ch = curl_init();
			curl_setopt($ch, CURLOPT_URL,$url);
			curl_setopt($ch, CURLOPT_HTTPHEADER,$header);
			curl_setopt($ch, CURLOPT_RETURNTRANSFER,TRUE);
			curl_setopt($ch, CURLOPT_POST,true);
			curl_setopt($ch, CURLOPT_POSTFIELDS,http_build_query($payload));
			curl_setopt($ch, CURLOPT_FAILONERROR,false);
			$body = json_decode(curl_exec($ch),true);
		}
	}
?>
