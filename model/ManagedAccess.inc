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
		
		/** @Collection(type=@String) */
		protected $options;
		
		/** @Collection(type=@Object(class='Renewal')) */
		protected $renewals;
		
		/** @String */
		protected $api;
		
		protected function getURI(){
			return "http://".SettingsManager::getSetting("install","urlroot")."/apis/{$this->getApi()}/users/{$this->getId()}";
		}
		
		protected function getURITemplate(){
			return "http://".SettingsManager::getSetting("install","urlroot")."/apis/{api_id}/users/{user_id}";
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
			
			foreach($this->getOptions() as $option){
				$eTag .= $option;
			}
		
			return md5($this->getModified().$eTag);
		}
	}
?>