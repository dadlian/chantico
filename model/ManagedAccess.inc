<?php
	class ManagedAccess extends Resource{
		/** @WadapiString */
		protected $username;
		
		/** @WadapiString */
		protected $authentication;
		
		/** @WadapiString(max=1024) */
		protected $accessKey;
		
		/** @WadapiString(max=1024) */
		protected $accessSecret;
		
		/** @WadapiString(max=1024) */
		protected $refreshSecret;
		
		/** @WadapiString(max=1024) */
		protected $accessEndpoint;
		
		/** @Integer */
		protected $expires;
		
		/** @Collection(type=@WadapiString) */
		protected $options;
		
		/** @Collection(type=@Object(class='Renewal')) */
		protected $renewals;
		
		/** @WadapiString */
		protected $api;
		
		/** @Boolean(default=false) */
		protected $multiSession;
		
		protected function getURI(){
			return $this->getBaseUri()."/apis/{$this->getApi()}/users/{$this->getId()}";
		}
		
		protected function getURITemplate(){
			return $this->getBaseUri()."/apis/{api_id}/users/{user_id}";
		}
		
		protected function getETag(){
			$eTag = $this->getUsername();
			$eTag .= $this->getAuthentication();
			$eTag .= $this->getAccessKey();
			$eTag .= $this->getAccessSecret();
			$eTag .= $this->getRefreshSecret();
			$eTag .= $this->getAccessEndpoint();
			$eTag .= $this->getExpires();
			$eTag .= $this->getApi();
		
			return md5($this->getModified().$eTag);
		}
	}
?>