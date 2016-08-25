<?php
	class API extends Resource{
		/** @URL */
		protected $root;
		
		/** @String */
		protected $accessKey;
		
		/** @String */
		protected $accessSecret;
		
		/** @Object(class='IPList') */
		protected $whitelist;
		
		/** @Object(class='IPList') */
		protected $blacklist;
		
		/** @Collection(type=@Object(class='ManagedAccess')) */
		protected $users;
		
		protected function getURI(){
			return ($_SERVER['HTTPS']?"https://":"http://").SettingsManager::getSetting("install","urlroot")."/apis/{$this->getId()}";
		}
		
		protected function getURITemplate(){
			return ($_SERVER['HTTPS']?"https://":"http://").SettingsManager::getSetting("install","urlroot")."/apis/{api_id}";
		}
		
		protected function getETag(){
			$eTag = $this->getRoot();
			foreach($this->getWhitelist() as $ip){
				$eTag.=$ip->getETag();
			}
			foreach($this->getBlacklist() as $ip){
				$eTag.=$ip->getETag();
			}
		
			return md5($this->getModified().$eTag);
		}
	}
?>