<?php
	class ManagedAccess extends Resource{
		/** @String */
		protected $username;
		
		/** @String */
		protected $authentication;
		
		/** @String */
		protected $accessKey;
		
		/** @String */
		protected $accessSecret;
		
		/** @String */
		protected $refreshSecret;
		
		/** @URL */
		protected $accessEndpoint;
		
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
			$eTag .= $this->getAccessURI();
		
			return md5($this->getModified().$eTag);
		}
	}
?>