<?php
	abstract class Controller extends WadadliClass{
		const DASH_ESCAPE = "84sh1";
	
		/** @Collection(type=@String) */
		protected $arguments;
		
		public function __construct(){
			call_user_func_array(array('parent','__construct'),func_get_args());
			Registrar::register($this);
			
			//Remove unspecified arguments
			$filteredArguments = array();
			foreach($this->getArguments() as $key => $value){
				if($value !== "@"){
					$filteredArguments[$key] = $value;
				}
			}
			$this->setArguments($filteredArguments);
		}
		
		protected function uploadFile($fileInformation){
			$mbSize = number_format($fileInformation['size']/1000,2);
			
			if($fileInformation['error'] == UPLOAD_ERR_OK){
				if($fileInformation['size'] > 1000000){
					warning("Maximum upload size is 1MB. Specified file is {$mbSize}KB","");
					return "";
				}
				
				$filename = time().preg_replace("/[\"']/","",$fileInformation['name']); 
				while(file_exists($uploadFilename = SettingsManager::getSetting("Install","Project Path")."/uploads/$filename")){ 
					$now++; 
				}

				move_uploaded_file($fileInformation['tmp_name'], $uploadFilename);
				return "uploads/$filename";
			}else{
				switch($fileInformation['error']){
					case UPLOAD_ERR_INI_SIZE: case UPLOAD_ERR_FORM_SIZE:
						warning("Maximum upload size is 1MB","");
						break;
					case UPLOAD_ERR_PARTIAL: case UPLOAD_ERR_NO_TMP_DIR:
						warning("The selected image was only partially uploaded.","");
						break;
					case UPLOAD_ERR_NO_FILE:
						break;
					default: 
						warning("There was an unspecified error in uploading your image.","");
						break;
				}
				
				return "";
			}
		}
		
		//Main execution loop of Controller, called by Dispatcher based on action.xml
		public abstract function execute();
		
		//Called at the beginning of the controller's execution, nothing happens unless method is redefined in child controller
		public function initialise(){
		}
		
		//Called at the end of the controller's execution, whether or not there are any errors during execution
		public function finish(){
		}
	}
?>