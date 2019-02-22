<?php
	use Wadapi\Http\Resource;
	use Wadapi\Persistence\SQLGateway;
	use Wadapi\Persistence\Searcher;
	use Wadapi\Persistence\Criterion;

	class UsernameRecovery extends Resource{
		protected function getURI(){
			$sqlGateway = new SQLGateway();
			$searcher = new Searcher();
			$searcher->addCriterion("usernameRecoveries",Criterion::INCLUDES,$this);
			$user = $sqlGateway->findUnique("ManagedAccess",$searcher);

			return $this->getBaseUri()."/users/{$user->getId()}/recoveries/{$this->getId()}";
		}

		protected function getURITemplate(){
			return $this->getBaseUri()."/users/{user_id}/recoveries/{recovery_id}";
		}

		protected function getETag(){
			return md5($this->getModified().$eTag);
		}
	}
?>
