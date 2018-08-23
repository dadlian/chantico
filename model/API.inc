<?php
	class API extends Resource{
		/** @URL */
		protected $root;

		/** @WadapiString */
		protected $accessKey;

		/** @WadapiString */
		protected $accessSecret;

		/** @WadapiObject(class='IPList') */
		protected $whitelist;

		/** @WadapiObject(class='IPList') */
		protected $blacklist;

		/** @Boolean */
		protected $secure;

		protected function getURI(){
			return $this->getBaseUri()."/apis/{$this->getId()}";
		}

		protected function getURITemplate(){
			return $this->getBaseUri()."/apis/{api_id}";
		}

		protected function getETag(){
			$eTag = $this->getRoot();
			$eTag.= $this->getAccessKey();
			$eTag.= $this->getAccessSecret();
			$eTag.= $this->getSecure();

			return md5($this->getModified().$eTag);
		}
	}
?>
