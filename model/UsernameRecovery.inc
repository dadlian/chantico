<?php
	use Wadapi\Http\Resource;

	class UsernameRecovery extends Resource{
		/** @WadapiString */
		protected $user;

		protected function getURI(){
			return $this->getBaseUri()."/users/{$this->getUser()}/recoveries/{$this->getId()}";
		}

		protected function getURITemplate(){
			return $this->getBaseUri()."/users/{user_id}/recoveries/{recovery_id}";
		}

		protected function getETag(){
			$eTag = $this->getUser();
			return md5($this->getModified().$eTag);
		}
	}
?>
