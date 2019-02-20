<?php
	use Wadapi\Http\ResponseHandler;
	use Wadapi\System\SettingsManager;
	use Wadapi\Persistence\SQLGateway;
	use Wadapi\Persistence\Searcher;
	use Wadapi\Persistence\Criterion;

	class UserCollection extends UserController{
		protected function post(){
			$normalisedUsername = strtolower($this->getFromContent("username"));

			$sqlGateway = new SQLGateway();
			$searcher = new Searcher();
			$searcher->addCriterion("username",Criterion::EQUAL,$normalisedUsername);
			if($sqlGateway->findUnique("ManagedAccess",$searcher)){
				ResponseHandler::conflict("A user identified by '$normalisedUsername' already exists.");
			}

			//Create New Access
			$payload = array_merge(array("role"=>$this->getFromContent("role")),$this->getFromContent("options",array()));
			$response = $this->sendInstancePost("/access",$payload);

			if($response['code'] == 400){
				ResponseHandler::bad(str_replace("arguments","directives",$response["body"]["message"]));
			}else if($response['code'] != 201){
				ResponseHandler::error("The server is misconfigured. Please contact a system administrator");
			}

			$options = array();
			if($response['code'] == 201){
				foreach($response["body"] as $option => $value){
					if(!in_array($option,array("self","tokens","active-token","role"))){
						$options[$option] = $value;
					}
				}
			}

			$instanceUrl = SettingsManager::getSetting("wadapi-instance","url");
			$accessEndpoint = preg_replace("/".preg_replace("/\//","\/",preg_quote($instanceUrl))."/","",$response["body"]["self"]);
			$tokensEndpoint = preg_replace("/".preg_replace("/\//","\/",preg_quote($instanceUrl))."/","",$response["body"]["tokens"]);

			$accessKey = $response["body"]["active-token"]["key"];
			$refreshSecret = $response["body"]["active-token"]["refresh"];

			//Refresh Access Token
			$response = $this->sendInstancePost($tokensEndpoint,array(),$accessKey,$refreshSecret);

			if($response['code'] !== 201){
				ResponseHandler::error("The server is misconfigured. Please contact a system administrator");
			}

			$accessKey = $response["body"]["key"];
			$accessSecret = $response["body"]["secret"];
			$refreshSecret = $response["body"]["refresh"];
			$lifetime = $response["body"]["lifetime"];

			//Save User to Chantico
			$user = new ManagedAccess();
			$user->setUsername($normalisedUsername);
			$user->setAuthentication(md5($this->getFromContent("authentication")));
			$user->setAccessKey($this->encrypt($accessKey,$this->getFromContent("authentication")));
			$user->setAccessSecret($this->encrypt($accessSecret,$this->getFromContent("authentication")));
			$user->setRefreshSecret($this->encrypt($refreshSecret,$this->getFromContent("authentication")));
			$user->setAccessEndpoint($accessEndpoint);
			$user->setExpires($lifetime?(time()+$lifetime):0);

			$optionsArray = array();
			foreach($this->getFromContent("options",$options) as $option => $value){
				$optionsArray[$option] = $value?json_encode($value):$value;
			}

			if(!array_key_exists("role",$optionsArray)){
				$optionsArray["role"] = $this->getFromContent("role");
			}

			$user->setOptions($optionsArray);
			$sqlGateway->save($user);

			//Create APIToken for User to Access its tokens
			$sqlGateway->save(new APIToken("user",0,md5($user->getUsername()),$user->getAuthentication(),$user->getAuthentication()));

			//Notify user of new Account
			$this->notify($user,$options);

			//Build Payload and Send Response
			$payload = array(
				"self"=>$user->getURI(),
				"username"=>$user->getUsername(),
				"role"=>$user->viewFromOptions("role"),
				"access"=>"{$instanceUrl}{$user->getAccessEndpoint()}",
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
			$notificationUrl = SettingsManager::getSetting("notifications","url");
			$notificationSender = SettingsManager::getSetting("notifications","sender");
			$templateFile = SettingsManager::getSetting("notifications","new-user")["template"];
			$templateSubject = SettingsManager::getSetting("notifications","new-user")["subject"];
			$templateTokens = SettingsManager::getSetting("notifications","new-user")["tokens"];

			$newAccountTemplate = file_get_contents(dirname(__FILE__)."/../../notifications/$templateFile");
			preg_replace("/\{\{username\}\}/",$user->getUsername(),$newAccountTemplate);
			preg_replace("/\{\{year\}\}/",date("Y"),$newAccountTemplate);

			foreach($templateTokens as $token){
				preg_replace("/\{\{$token\}\}/",$options[$token],$newAccountTemplate);
			}

			//Replace Template Tokens
			$firstName = array_key_exists('first-name',$options)?$options['first-name']:$user->getUsername();
			$lastName = array_key_exists('last-name',$options)?$options['last-name']:"";
			$email = array_key_exists('email',$options)?$options['email']:"";
			$recipient = "$firstName $lastName $email";

			$payload = array(
				"to"=>$recipient,
				"from"=> $notificationSender,
				"subject"=>$templateSubject,
				"html"=> $newAccountTemplate
			);

			$header = array(
				'Content-Type: application/x-www-form-urlencoded'
			);

			$ch = curl_init($notificationUrl);
			curl_setopt($ch, CURLOPT_HTTPHEADER,$header);
			curl_setopt($ch, CURLOPT_RETURNTRANSFER,TRUE);
			curl_setopt($ch, CURLOPT_POST,true);
			curl_setopt($ch, CURLOPT_POSTFIELDS,http_build_query($payload));
			curl_setopt($ch, CURLOPT_FAILONERROR,false);
			json_decode(curl_exec($ch),true);
			curl_close($ch);
		}
	}
?>
