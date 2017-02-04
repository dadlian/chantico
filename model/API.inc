<?php
	class API extends Resource{
		/** @URL */
		protected $root;
		
		/** @WadapiString */
		protected $accessKey;
		
		/** @WadapiString */
		protected $accessSecret;
		
		/** @Object(class='IPList') */
		protected $whitelist;
		
		/** @Object(class='IPList') */
		protected $blacklist;
		
		/** @Collection(type=@Object(class='ManagedAccess')) */
		protected $users;
		
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