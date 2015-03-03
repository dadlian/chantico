<?php
	class FatalError extends Controller{
		public function execute(){
			$errorPage = new PageTemplate();
			$errorPage->setView(new ErrorView());
			$errorPage->render();
		}
	}
?>