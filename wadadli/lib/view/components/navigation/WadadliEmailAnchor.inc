<?php
	class WadadliEmailAnchor extends WadadliAnchor{
		public function setLink($newEmail){
			$this->link = $newEmail;
			$this->addAttribute('href',"mailto:$newEmail");
		}
	}
?>