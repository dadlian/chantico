<?php
	class UsernameRecovery extends Resource{
		/** @String */
		protected $user;
		
		protected function getURI(){
			$sqlGateway = new SQLGateway();
			$searcher = new Searcher();
			$searcher->addCriterion("id",Criterion::EQUAL,$this->getUser());
			$user = $sqlGateway->findUnique("ManagedAccess",$searcher);
			
			return $this->getBaseUri()."/apis/{$user->getApi()}/users/{$this->getUser()}/recoveries/{$this->getId()}";
		}
		
		protected function getURITemplate(){
			return $this->getBaseUri()."/apis/{api_id}/users/{user_id}/recoveries/{recovery_id}";
		}
		
		protected function getETag(){
			$eTag = $this->getUser();
			return md5($this->getModified().$eTag);
		}
	}
?>