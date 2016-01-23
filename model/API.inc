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
		
		protected function getURI(){
			return "http://".SettingsManager::getSetting("install","urlroot")."/apis/{$this->getId()}";
		}
		
		protected function getURITemplate(){
			return "http://".SettingsManager::getSetting("install","urlroot")."/apis/{api_id}";
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