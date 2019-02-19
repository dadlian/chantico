<?php
	use Wadapi\Http\ResponseHandler;
	use Wadapi\Http\PagedResourceController;

	abstract class ChanticoController extends PagedResourceController{
		public function execute(){
			$blacklist = $this->getResourceObject("IPList","title","blacklist");
			$whitelist = $this->getResourceObject("IPList","title","whitelist");

			if($blacklist && in_array($_SERVER['REMOTE_ADDR'],$blacklist->getEntries())){
				ResponseHandler::forbidden("The originating IP address is not permitted to make requests.");
			}

			if($whitelist && $whitelist->getEntries() && !in_array($_SERVER['REMOTE_ADDR'],$whitelist->getEntries())){
				ResponseHandler::forbidden("The originating IP address is not permitted to make requests.");
			}

			parent::execute();
		}

		protected function getInvalidQueryParameters(){
		}

		protected function getRecordCount(){
		}

		protected function buildPageEntries($start,$count){
		}
	}
?>
