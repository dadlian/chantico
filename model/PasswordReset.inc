<?php
	use Wadapi\Http\Resource;
	use Wadapi\Persistence\SQLGateway;
	use Wadapi\Persistence\Searcher;
	use Wadapi\Persistence\Criterion;

	class PasswordReset extends Resource{
		/** @Integer */
		protected $code;

		/** @WadapiString(values={'requested','reset','cancelled'}) */
		protected $status;

		protected function getURI(){
			$sqlGateway = new SQLGateway();
			$searcher = new Searcher();
			$searcher->addCriterion("passwordResets",Criterion::INCLUDES,$this);
			$user = $sqlGateway->findUnique("ManagedAccess",$searcher);

			return $this->getBaseUri()."/users/{$user->getId()}/resets/{$this->getId()}";
		}

		protected function getURITemplate(){
			return $this->getBaseUri()."/users/{user_id}/resets/{reset_id}";
		}

		protected function getETag(){
			$eTag = $this->getCode();
			$eTag.= $this->getStatus();

			return md5($this->getModified().$eTag);
		}
	}
?>
