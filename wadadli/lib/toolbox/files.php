<?php	
	function require_all($directory){
		if(!file_exists($directory)){
			return;
		}else if(is_dir($directory)){	
			RequireUtil::require_directory($directory);
		}else{	
			RequireUtil::require_file($directory);
		}
	}
	
	function backup_file($filepath){
		if(!file_exists($filepath)){
			warning(RESOURCE_UNAVAILABLE_ERROR,"There was an attempt to backup a non-existant file: $filepath");
			return;
		}
		
		if(file_exists("$filepath.bk")){
			warning("","There was an attempt to backup a file for which a backup already exists: $filepath");
			return;		
		}
		
		copy($filepath, "$filepath.bk");
	}
	
	function restore_file($filepath){
		if(!file_exists($filepath)){
			warning(RESOURCE_UNAVAILABLE_ERROR,"There was an attempt to restore a non-existant file: $filepath");
			return;
		}
		
		if(!file_exists("$filepath.bk")){
			warning("","There was an attempt to restore a file that has no backup: $filepath");
			return;
		}
		
		copy("$filepath.bk", $filepath);
		unlink("$filepath.bk");	
	}
	
	class RequireUtil{
		private static $includedClasses = array();
		
		public static function require_directory($directoryString){
			$directory = opendir($directoryString);
			$directoryContents = array();

			while(($filename = readdir($directory)) !== FALSE) {
				$directoryContents[] = $directoryString."/".$filename;
			}
			
			foreach($directoryContents as $content){
				if(is_file($content) && (substr($content, strlen($content) - 3) == "inc" || substr($content, strlen($content) - 3) == "php")){
					self::require_file($content);
				}else if (is_dir($content) && substr($content, strlen($content) - 1) !== "." && substr($content, strlen($content) - 2) !== ".."){
					self::require_directory($content);
				}
			}

			closedir($directory);
		}
	
		public static function require_file($filename){
			if(!file_exists($filename)){
				fatal_error(RESOURCE_UNAVAILABLE_ERROR,"require_all() could not open file: $filename");
				return;
			}
			
			//See if file contains PHP class or not
			$fileContents = file_get_contents($filename);
			preg_match("/class\s+(\w+)/", $fileContents, $matches);
			
			if(sizeof($matches) > 1){
				$fileParts = preg_split("/\//",$filename);
				$filePath = str_replace($fileParts[sizeof($fileParts)-1],"",$filename);
				self::require_class($matches[1], $filePath);
			}else{		
				require_once($filename);
			}
		}

		private static function require_class($className, $directory){			
			//Ignore class if it has already been included
			if(in_array($className,self::$includedClasses)){
				return;
			}
			
			if(!$includeFilename = self::find_class_file($className,$directory)){
				if(!$includeFilename = self::find_class_file($className,dirname(__FILE__)."/../..")){
					return false;
				}
			}
			
			//Check whether or not the class has a parent, and include the ancestor first if so
			$classText = file_get_contents($includeFilename);
			preg_match("/class\s+$className\s+extends\s+(\w+)/", $classText, $matches);		
			if(sizeof($matches) > 1){
				self::require_class($matches[1],$directory);
			}

			require_once($includeFilename);	
			self::$includedClasses[] = $className;
		}

		private static function find_class_file($className, $directoryName){
			$children = array($directoryName);
			
			while(sizeof($children) > 0){
				$directoryName = array_shift($children);
				$directory = opendir($directoryName);

				while(($nextFile = readdir($directory)) !== FALSE){
					$filePath = "$directoryName/$nextFile"; 
					if(is_file($filePath)){
						$fileContents = file_get_contents($filePath);
						preg_match("/class\s+($className)[\s{]/", $fileContents, $matches);
						
						if(sizeof($matches) > 1){
							return $filePath;
						}
					}else if($nextFile !== "." && $nextFile !== ".."){
						array_push($children, $filePath);
					}
				}
				
				closedir($directory);
			}
			
			return false;
		}	
	}
?>