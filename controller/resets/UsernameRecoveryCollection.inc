<?php
	use Wadapi\Http\ResponseHandler;
	use Wadapi\System\SettingsManager;
	use Wadapi\Persistence\SQLGateway;;
	use Wadapi\Persistence\Searcher;

	class UsernameRecoveryCollection extends UsernameRecoveryController{
		protected function post(){
			$sqlGateway = new SQLGateway();
			$searcher = new Searcher();
			$user = $this->getResourceObject("ManagedAccess","id",$this->viewFromArguments("user"));

			//Create New Username Recovery Request
			$recovery = new UsernameRecovery();
			$recovery->setUser($this->viewFromArguments("user"));
			$sqlGateway->save($recovery);

			//Send Confirmation Code via Email
			$notificationUrl = SettingsManager::getSetting("notifications","url");
			$notificationSender = SettingsManager::getSetting("notifications","sender");
			$templateFile = SettingsManager::getSetting("notifications","username-recovery")["template"];
			$templateSubject = SettingsManager::getSetting("notifications","username-recovery")["subject"];

			if($templateFile){
				//Replace Template Tokens
				$usernameRecoveryTemplate = file_get_contents(dirname(__FILE__)."/../../notifications/$templateFile");
				$usernameRecoveryTemplate = preg_replace("/\{\{recipient\}\}/",$this->getFromContent("recipient"),$usernameRecoveryTemplate);
				$usernameRecoveryTemplate = preg_replace("/\{\{username\}\}/",$user->getUsername(),$usernameRecoveryTemplate);
				$usernameRecoveryTemplate = preg_replace("/\{\{year\}\}/",date("Y"),$usernameRecoveryTemplate);

				$recipient = "{$this->getFromContent('recipient')} <{$this->getFromContent('email')}>";

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
