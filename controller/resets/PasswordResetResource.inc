<?php
	use Wadapi\Http\ResponseHandler;
	use Wadapi\Persistence\SQLGateway;
	use Wadapi\Persistence\Searcher;
	use Wadapi\Persistence\Criterion;
	use Wadapi\System\SettingsManager;

	class PasswordResetResource extends PasswordResetController{
		protected function get(){
			$user = $this->getResourceObject("ManagedAccess","id",$this->viewFromArguments("user"));
			$reset = $this->getResourceObject("PasswordReset","id",$this->viewFromArguments("reset"));

			$payload = $this->assemblePayload($reset);
			ResponseHandler::retrieved($payload,$reset->getURI(),$reset->getModified(),$reset->getETag());
		}

		protected function put(){
			$user = $this->getResourceObject("ManagedAccess","id",$this->viewFromArguments("user"));
			$reset = $this->getResourceObject("PasswordReset","id",$this->viewFromArguments("reset"));

			if(in_array($reset->getStatus(),array("reset","cancelled"))){
				ResponseHandler::conflict("The password reset request has been resolved, and can no longer be modified.");
			}

			if($reset->getCode() !== $this->getFromContent("code")){
				ResponseHandler::forbidden("The given password reset code is invalid for this request.");
			}

			//Find User Options
			$role = json_decode($user->viewFromOptions("role"));
			if(json_last_error() != JSON_ERROR_NONE){
				$role = $user->viewFromOptions("role");
			}

			$options = array();
			foreach($user->getOptions() as $option => $value){
				if($option != "role"){
					$options[$option] = json_decode($value);
					if(json_last_error() != JSON_ERROR_NONE){
						$options[$option] = $value;
					}
				}
			}

			$optionsInitialised = sizeof($options) > 0;
			foreach($options as $option){
				if(!$option){
					$optionsInitialised = false;
					break;
				}
			}

			if(!$optionsInitialised){
				$response = $this->sendInstanceGet($user->getAccessEndpoint());
				$role = $response["body"]["role"];

				foreach($response["body"] as $option => $value){
					if(!in_array($option,array("self","tokens","active-token","role"))){
						$options[$option] = $value;
					}
				}
			}

			//Echo old access endpoint
			$instanceUrl = SettingsManager::getSetting("wadapi-instance","url");
			$options['predecessor'] = $instanceUrl.$user->getAccessEndpoint();

			//Create Fresh Access Tokens
			$payload = json_encode(array_merge(array("role"=>$role),$options));
			$response = $this->sendInstancePost("/access",$payload);

			if($response['code'] == 400){
				ResponseHandler::bad(str_replace("arguments","directives",$response['body']["message"]));
			}else if($response['code'] != 201){
				ResponseHandler::error("The server is misconfigured. Please contact a system administrator");
			}

			$accessEndpoint = parse_url($response['body']["self"],PHP_URL_PATH);
			$tokensEndpoint = parse_url($response['body']["tokens"],PHP_URL_PATH);
			$accessKey = $response['body']["active-token"]["key"];
			$refreshSecret = $response['body']["active-token"]["refresh"];

			//Refresh Access Token
			$response = $this->sendInstancePost($tokensEndpoint,array(),$accessKey,$refreshSecret);

			if($response['code'] != 201){
				ResponseHandler::error("The server is misconfigured. Please contact a system administrator");
			}

			//Encrypt and save new access tokens
			$accessKey = $response["body"]["key"];
			$accessSecret = $response["body"]["secret"];
			$refreshSecret = $response["body"]["refresh"];
			$lifetime = $response["body"]["lifetime"];

			//Save user changes to Chantico
			$authentication = $this->getFromContent("authentication");
			$user->setAuthentication(md5($authentication));
			$user->setAccessKey($this->encrypt($accessKey,$authentication));
			$user->setAccessSecret($this->encrypt($accessSecret,$authentication));
			$user->setRefreshSecret($this->encrypt($refreshSecret,$authentication));
			$user->setAccessEndpoint($accessEndpoint);
			$user->setExpires(time()+$lifetime);
			$user->setOptions($user->getOptions());

			$sqlGateway = new SQLGateway();
			$sqlGateway->save($user);

			//Get user's APIToken
			$searcher = new Searcher();
			$searcher->addCriterion("accessKey",Criterion::EQUAL,md5($user->getUsername()));
			$apiToken = $sqlGateway->findUnique("APIToken",$searcher);

			//Update APIToken for User to Access its
			$apiToken->setAccessKey(md5($user->getUsername()));
			$apiToken->setAccessSecret($user->getAuthentication());
			$apiToken->setRefreshSecret($user->getAuthentication());
			$sqlGateway->save($apiToken);

			//Update Password Reset Status
			$reset->setStatus("reset");
			$sqlGateway->save($reset);

			$payload = $this->assemblePayload($reset);
			ResponseHandler::modified($payload,$reset->getURI(),$reset->getModified(),$reset->getETag());
		}

		protected function isConsistent($modifiedDate,$eTag){
			$reset = $this->getResourceObject("PasswordReset","id",$this->viewFromArguments("reset"));
			return $modifiedDate == $reset->getModified() && $eTag == $reset->getETag();
		}

		protected function encrypt($data,$key){
			$key = md5(password_hash($key,PASSWORD_DEFAULT,array("salt"=>SettingsManager::getSetting("encryption","salt"))));
			$key = pack('H*',$key);

			# create a random IV to use with CBC encoding
			$iv_size = mcrypt_get_iv_size(MCRYPT_RIJNDAEL_128, MCRYPT_MODE_CBC);
			$iv = mcrypt_create_iv($iv_size, MCRYPT_RAND);

			# creates a cipher text compatible with AES (Rijndael block size = 128)
			$ciphertext = $iv . mcrypt_encrypt(MCRYPT_RIJNDAEL_128, $key, $data, MCRYPT_MODE_CBC, $iv);
			return base64_encode($ciphertext);
		}
	}
?>
