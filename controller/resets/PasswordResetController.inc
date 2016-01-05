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
			$count = 0;
			if(DatabaseAdministrator::tableExists("PasswordReset")){
				$user = $this->viewFromArguments("user");
				$countResult = DatabaseAdministrator::execute("SELECT COUNT(id) as reset FROM PasswordReset WHERE user = $user");
				$count = $countResult[0]["reset"];
			}
			
			return $count;
		}
		
		protected function buildPageEntries($start,$count){
			$payload = array();
			$sqlGateway = new SQLGateway();
			$searcher = new Searcher();
			$searcher->addCriterion("user",Criterion::EQUAL,$this->viewFromArguments("user"));
			$sorter = new Sorter();
			$sorter->addCriterion("id",Criterion::DESCENDING);
			
			foreach($sqlGateway->find("PasswordReset",$searcher,$sorter,$count,$start) as $reset){
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