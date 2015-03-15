<?php
	class APIToken extends Resource{
		/** @String(required=true) */
		protected $role;
		
		/** @Integer */
		protected $expires;
		
		/** @String(max=32) */
		protected $accessKey;
		
		/** @String(max=32) */
		protected $accessSecret;
		
		/** @String(max=32) */
		protected $refreshSecret;
		
		protected function refresh($lifetime=0){
			$accessKey = md5((time()/rand()) * rand());
			$accessSecret = md5((time()/rand()) * rand());
			$refreshSecret = md5((time()/rand()) * rand());
		
			$this->setAccessKey(md5($accessKey));
			$this->setAccessSecret(md5($accessSecret));
			$this->setRefreshSecret(md5($refreshSecret));
			$this->setExpires($lifetime?$this->getModified()+$lifetime:$lifetime);
			
			return array("key"=>$accessKey,"secret"=>$accessSecret,"refresh"=>$refreshSecret);
		}
		
		protected function getURI(){
			return "http://".SettingsManager::getSetting("install","urlroot")."/access/{$this->getId()}";
		}
		
		protected function getURITemplate(){
			return "http://".SettingsManager::getSetting("install","urlroot")."/access/{access_id}";
		}
		
		protected function getETag(){
			$eTag = $this->getRole();
			$eTag .= $this->getExpires();
			$eTag .= $this->getAccessKey();
			$eTag .= $this->getAccessSecret();
			$eTag .= $this->getRefreshSecret();
			return md5($this->getModified().$eTag);
		}
	}
?>