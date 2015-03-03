<?php
	abstract class Resource extends PersistentClass{
		abstract protected function getURI();
		abstract protected function getURITemplate();
		abstract protected function getETag();
	}
?>