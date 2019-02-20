<?php
	use Wadapi\Http\Resource;

	class PasswordReset extends Resource{
		/** @WadapiString */
		protected $user;

		/** @Integer */
		protected $code;

		/** @WadapiString(values={'requested','reset','cancelled'}) */
		protected $status;

		protected function getURI(){
			return $this->getBaseUri()."/users/{$this->getUser()}/resets/{$this->getId()}";
		}

		protected function getURITemplate(){
			return $this->getBaseUri()."/users/{user_id}/resets/{reset_id}";
		}

		protected function getETag(){
			$eTag = $this->getUser();
			$eTag.= $this->getCode();
			$eTag.= $this->getStatus();

			return md5($this->getModified().$eTag);
		}
	}
?>
