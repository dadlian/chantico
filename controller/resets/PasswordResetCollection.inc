<?php
	use Wadapi\Http\ResponseHandler;
	use Wadapi\Persistence\SQLGateway;
	use Wadapi\System\SettingsManager;

	class PasswordResetCollection extends PasswordResetController{
		protected function post(){
			$user = $this->getResourceObject("ManagedAccess","id",$this->viewFromArguments("user"));

			//Cancel previously requested password resets
			foreach($user->getPasswordResets() as $passwordReset){
				if($passwordReset->getStatus() == "requested"){
					$passwordReset->setStatus("cancelled");
				}
			}

			//Create New Password Reset Request
			$reset = new PasswordReset();
			$reset->setCode(rand(100000,999999));
			$reset->setStatus("requested");
			$user->appendToPasswordResets($reset);

			$sqlGateway = new SQLGateway();
			$sqlGateway->save($user);

			//Send Confirmation Code via Email
			$notificationUrl = SettingsManager::getSetting("notifications","url");
			$notificationSender = SettingsManager::getSetting("notifications","sender");
			$templateFile = SettingsManager::getSetting("notifications","password-reset")["template"];
			$templateSubject = SettingsManager::getSetting("notifications","password-reset")["subject"];

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

			if($templateFile && $userEmail){
				//Replace Template Tokens
				$passwordResetTemplate = file_get_contents(dirname(__FILE__)."/../../notifications/$templateFile");
				$passwordResetTemplate = preg_replace("/\{\{first-name\}\}/",$userFirstName,$passwordResetTemplate);
				$passwordResetTemplate = preg_replace("/\{\{last-name\}\}/",$userLastName,$passwordResetTemplate);
				$passwordResetTemplate = preg_replace("/\{\{email\}\}/",$userEmail,$passwordResetTemplate);
				$passwordResetTemplate = preg_replace("/\{\{recipient\}\}/",$recipientName,$passwordResetTemplate);
				$passwordResetTemplate = preg_replace("/\{\{code\}\}/",$reset->getCode(),$passwordResetTemplate);
				$passwordResetTemplate = preg_replace("/\{\{year\}\}/",date("Y"),$passwordResetTemplate);

				$recipient = "$recipientName <$userEmail>";

				$payload = array(
					"to"=>$recipient,
					"from"=> $notificationSender,
					"subject"=>$templateSubject,
					"html"=> $passwordResetTemplate
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

			$payload = $this->assemblePayload($reset);
			ResponseHandler::created($payload,$reset->getURI());
		}

		protected function isConsistent($modifiedDate,$eTag){
			return true;
		}
	}
?>
