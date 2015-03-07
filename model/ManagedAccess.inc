<?php
	class ManagedAccess extends Resource{
		/** @String */
		protected $username;
		
		/** @String */
		protected $authentication;
		
		/** @String(max=1024) */
		protected $accessKey;
		
		/** @String(max=1024) */
		protected $accessSecret;
		
		/** @String(max=1024) */
		protected $refreshSecret;
		
		/** @String(max=1024) */
		protected $accessEndpoint;
		
		/** @Integer */
		protected $expires;
		
		protected function getURI(){
			return "http://".SettingsManager::getSetting("install","urlroot")."/users/{$this->getId()}";
		}
		
		protected function getURITemplate(){
			return "http://".SettingsManager::getSetting("install","urlroot")."/users/{user_id}";
		}
		
		protected function getETag(){
			$eTag = $this->getUsername();
			$eTag .= $this->getAuthentication();
			$eTag .= $this->getAccessKey();
			$eTag .= $this->getAccessSecret();
			$eTag .= $this->getRefreshSecret();
			$eTag .= $this->getAccessEndpoint();
			$eTag .= $this->getExpires();
		
			return md5($this->getModified().$eTag);
		}
	}
?>