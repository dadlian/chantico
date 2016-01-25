<?php
	class PasswordReset extends Resource{
		/** @String */
		protected $user;
		
		/** @Integer */
		protected $code;
		
		/** @String(values={'requested','reset','cancelled'}) */
		protected $status;
		
		protected function getURI(){
			$sqlGateway = new SQLGateway();
			$searcher = new Searcher();
			$searcher->addCriterion("id",Criterion::EQUAL,$this->getUser());
			$user = $sqlGateway->findUnique("ManagedAccess",$searcher);
			
			return "http://".SettingsManager::getSetting("install","urlroot")."/apis/{$user->getApi()}/users/{$this->getUser()}/resets/{$this->getId()}";
		}
		
		protected function getURITemplate(){
			return "http://".SettingsManager::getSetting("install","urlroot")."/apis/{api_id}/users/{user_id}/resets/{reset_id}";
		}
		
		protected function getETag(){
			$eTag = $this->getUser();
			$eTag.= $this->getCode();
			$eTag.= $this->getStatus();
		
			return md5($this->getModified().$eTag);
		}
	}
?>