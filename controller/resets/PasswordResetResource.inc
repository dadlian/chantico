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
			$userOptions = array();
			foreach($user->getOptions() as $option => $value){
				$userOptions[$option] = json_decode($value);
				if(json_last_error() != JSON_ERROR_NONE){
					$userOptions[$option] = $value;
				}
			}

			//Create Fresh Access Tokens
			$payload = $userOptions;
			$instanceUrl = SettingsManager::getSetting("wadapi-instance","url");

			$payload['invalidated-token'] = $instanceUrl.$user->getAccessEndpoint();
			$response = $this->sendInstancePost("access",$payload);

			if($response['code'] == 400){
				ResponseHandler::bad(str_replace("arguments","directives",$response["body"]["message"]));
			}else if($response['code'] != 201){
				ResponseHandler::error("The server is misconfigured. Please contact a system administrator");
			}

			//Delete old token
			$this->sendInstanceDelete($user->getAccessEndpoint());foreach($response['body']['profile'] as $option => $value){
				$user->insertToOptions($option,json_encode($value));
			}

			$accessEndpoint = preg_replace("/".preg_replace("/\//","\/",preg_quote($instanceUrl))."/","",$response["body"]["self"]);
			$tokensEndpoint = preg_replace("/".preg_replace("/\//","\/",preg_quote($instanceUrl))."/","",$response["body"]["tokens"]);
			$accessKey = $response['body']["active-token"]["key"];
			$refreshSecret = $response['body']["active-token"]["refresh"];

			//Refresh Access Token
			$response = $this->sendInstancePost($tokensEndpoint,array(),$accessKey,$refreshSecret);

			if($response['code'] != 201){
				ResponseHandler::error("The server is misconfigured. Please contact a system administrator");
			}

			$accessKey = $response['body']["key"];
			$accessSecret = $response['body']["secret"];
			$refreshSecret = $response['body']["refresh"];
			$lifetime = $response['body']["lifetime"];

			$user->setAuthentication(md5($this->getFromContent("authentication")));
			$user->setAccessKey($this->encrypt($accessKey,$this->getFromContent("authentication")));
			$user->setAccessSecret($this->encrypt($accessSecret,$this->getFromContent("authentication")));
			$user->setRefreshSecret($this->encrypt($refreshSecret,$this->getFromContent("authentication")));
			$user->setAccessEndpoint($accessEndpoint);
			$user->setExpires(time()+$lifetime);

			$sqlGateway = new SQLGateway();
			$sqlGateway->save($user);

			//Get user's APIToken
			$searcher = new Searcher();
			$searcher->addCriterion("accessKey",Criterion::EQUAL,md5($user->getUsername()));
			$apiToken = $sqlGateway->findUnique("Wadapi\Authentication\APIToken",$searcher);

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
