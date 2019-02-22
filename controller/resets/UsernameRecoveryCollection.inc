<?php
	use Wadapi\Http\ResponseHandler;
	use Wadapi\System\SettingsManager;
	use Wadapi\Persistence\SQLGateway;

	class UsernameRecoveryCollection extends UsernameRecoveryController{
		protected function post(){
			$user = $this->getResourceObject("ManagedAccess","id",$this->viewFromArguments("user"));

			//Create New Username Recovery Request
			$sqlGateway = new SQLGateway();

			$recovery = new UsernameRecovery();
			$sqlGateway->save($recovery);

			$user->appendToUsernameRecoveries($recovery);
			$sqlGateway->save($user);

			//Send Confirmation Code via Email
			$notificationUrl = SettingsManager::getSetting("notifications","url");
			$notificationSender = SettingsManager::getSetting("notifications","sender");
			$templateFile = SettingsManager::getSetting("notifications","username-recovery")["template"];
			$templateSubject = SettingsManager::getSetting("notifications","username-recovery")["subject"];

			//Load User Information if available
			$userFirstName = "";
			if($user->viewFromOptions("first-name")){
				$userFirstName = json_decode($user->viewFromOptions("first-name"));
				if(json_last_error() != JSON_ERROR_NONE){
					$userFirstName = $user->viewFromOptions("first-name");
				}
			}

			$userLastName = "";
			if($user->viewFromOptions("last-name")){
				$userLastName = json_decode($user->viewFromOptions("last-name"));
				if(json_last_error() != JSON_ERROR_NONE){
					$userLastName = $user->viewFromOptions("last-name");
				}
			}

			$userEmail = "";
			if($user->viewFromOptions("email")){
				$userEmail = json_decode($user->viewFromOptions("email"));
				if(json_last_error() != JSON_ERROR_NONE){
					$userEmail = $user->viewFromOptions("email");
				}
			}

			$recipientName = "$userFirstName $userLastName";

			if($templateFile){
				//Replace Template Tokens
				$usernameRecoveryTemplate = file_get_contents(dirname(__FILE__)."/../../notifications/$templateFile");
				$usernameRecoveryTemplate = preg_replace("/\{\{first-name\}\}/",$userFirstName,$usernameRecoveryTemplate);
				$usernameRecoveryTemplate = preg_replace("/\{\{last-name\}\}/",$userLastName,$usernameRecoveryTemplate);
				$usernameRecoveryTemplate = preg_replace("/\{\{email\}\}/",$userEmail,$usernameRecoveryTemplate);
				$usernameRecoveryTemplate = preg_replace("/\{\{recipient\}\}/",$recipientName,$usernameRecoveryTemplate);
				$usernameRecoveryTemplate = preg_replace("/\{\{username\}\}/",$user->getUsername(),$usernameRecoveryTemplate);
				$usernameRecoveryTemplate = preg_replace("/\{\{year\}\}/",date("Y"),$usernameRecoveryTemplate);

				$recipient = "$recipientName <$userEmail>";

				$payload = array(
					"to"=>$recipient,
					"from"=> $notificationSender,
					"subject"=>$templateSubject,
					"html"=> $usernameRecoveryTemplate
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

			$payload = $this->assemblePayload($recovery);
			ResponseHandler::created($payload,$recovery->getURI());
		}

		protected function isConsistent($modifiedDate,$eTag){
			return true;
		}
	}
?>
