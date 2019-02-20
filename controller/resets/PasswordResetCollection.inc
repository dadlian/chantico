<?php
	use Wadapi\Http\ResponseHandler;
	use Wadapi\Persistence\DatabaseAdministrator;
	use Wadapi\Persistence\SQLGateway;
	use Wadapi\Persistence\Searcher;
	use Wadapi\Persistence\Criterion;
	use Wadapi\System\SettingsManager;

	class PasswordResetCollection extends PasswordResetController{
		protected function post(){
			$sqlGateway = new SQLGateway();
			$searcher = new Searcher();

			//Close any open reset requests
			$searcher->addCriterion("status",Criterion::EQUAL,"requested");
			$searcher->addCriterion("user",Criterion::EQUAL,$this->viewFromArguments("user"));
			$openResets = array();
			foreach($sqlGateway->find("PasswordReset",$searcher) as $openReset){
				$openReset->setStatus("cancelled");
				$openResets[] = $openReset;
			}
			$sqlGateway->save($openResets);

			//Create New Password Reset Request
			$reset = new PasswordReset();
			$reset->setUser($this->viewFromArguments("user"));
			$reset->setCode(rand(100000,999999));
			$reset->setStatus("requested");
			$sqlGateway->save($reset);

			$searcher->clearCriteria();
			$searcher->addCriterion("id",Criterion::EQUAL,$reset->getUser());
			$managedAccess = $sqlGateway->findUnique("ManagedAccess",$searcher);

			//Send Confirmation Code via Email
			$notificationUrl = SettingsManager::getSetting("notifications","url");
			$notificationSender = SettingsManager::getSetting("notifications","sender");
			$templateFile = SettingsManager::getSetting("notifications","password-reset")["template"];
			$templateSubject = SettingsManager::getSetting("notifications","password-reset")["subject"];

			//Replace Template Tokens
			$passwordResetTemplate = file_get_contents(dirname(__FILE__)."/../../notifications/$templateFile");
			preg_replace("/\{\{recipient\}\}/",$this->getFromContent("recipient"),$passwordResetTemplate);
			preg_replace("/\{\{code\}\}/",$reset->getCode(),$passwordResetTemplate);
			preg_replace("/\{\{year\}\}/",date("Y"),$passwordResetTemplate);

			$recipient = "{$this->getFromContent('recipient')} <{$this->getFromContent('email')}>";

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

			$payload = $this->assemblePayload($reset);
			ResponseHandler::created($payload,$reset->getURI());
		}

		protected function isConsistent($modifiedDate,$eTag){
			return true;
		}
	}
?>
