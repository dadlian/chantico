<?php
	abstract class PasswordResetController extends ChanticoController{
		protected function isInvalid(){
			$invalidArguments = array();

			$recipient = $this->getFromContent("recipient");
			if($recipient && !is_string($recipient)){
				$invalidArguments[] = "recipient";
			}

			$email = $this->getFromContent("email");
			if($email && !filter_var($email,FILTER_VALIDATE_EMAIL)){
				$invalidArguments[] = "email";
			}

			$authentication = $this->getFromContent("authentication","");
			if($authentication && !is_string($authentication)){
				$invalidArguments[] = "authentication";
			}

			$code = $this->getFromContent("code","");
			if($code && (!is_int($code) || $code < 100000 || $code > 999999)){
				$invalidArguments[] = "code";
			}

			return $invalidArguments;
		}

		protected function getInvalidQueryParameters(){
			$invalidQueryParameters = array();
			return $invalidQueryParameters;
		}

		protected function getRecordCount(){
			$user = $this->getResourceObject("ManagedAccess","id",$this->viewFromArguments("user"));
			$count = sizeof($user->getPasswordResets());

			return $count;
		}

		protected function buildPageEntries($start,$count){
			$user = $this->getResourceObject("ManagedAccess","id",$this->viewFromArguments("user"));
			$resets = array();
			foreach($user->getPasswordResets() as $reset){
				$resets[$reset->getId()] = $reset;
			}

			krsort($resets);
			foreach(array_slice($resets,$start,$count) as $reset){
				$payload[] = $this->assemblePayload($reset);
			}

			return $payload;
		}

		protected function assemblePayload($reset){
			$closed = in_array($reset->getStatus(),array("reset","cancelled"))?date("c",$reset->getModified()):"";
			$payload = array(
				"self"=>$reset->getURI(),
				"status"=>$reset->getStatus(),
				"requested"=>date("c",$reset->getCreated()),
				"closed"=>$closed
			);

			return $payload;
		}
	}
?>
