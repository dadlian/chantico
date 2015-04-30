<?php
	class IPList extends Resource{
		/** @String */
		protected $title;
		
		/** @Collection(type=@String) */
		protected $entries;
		
		protected function getURI(){
			return "/{$this->getTitle()}";
		}
		
		protected function getURITemplate(){
			return "/{$this->getTitle()}";
		}
		
		protected function getETag(){
			$eTag = $this->getTitle();
			foreach($this->getEntries() as $entry){
				$eTag .= $entry;
			}
		
			return md5($this->getModified().$eTag);
		}
	}
?>