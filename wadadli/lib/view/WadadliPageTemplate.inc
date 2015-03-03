<?php
	class WadadliPageTemplate extends PersistentClass{
		/** @String(default='Wadadli Page') */
		protected $title;
		
		/** @Object(class='View') */
		protected $view;
		
		private $favicon;
		
		private $loadAction;
		
		private $loadArguments;
		
		private $meta;
		
		private $stylesheets;
		
		private $headerScripts;
		private $footerScripts;
		
		private $urlRoot;
		
		public function __construct(){
			call_user_func_array(array('parent','__construct'), func_get_args());
			
			$this->urlRoot = SettingsManager::getSetting('Install','Url Root')."/";
			
			if(!preg_match("/^https?:\/\//",$this->urlRoot)){
				$this->urlRoot = "http://{$this->urlRoot}";
			}
			
			$this->meta = array();
			$this->stylesheets = array();
			$this->headerScripts = array();
			$this->footerScripts = array();
			
			$this->addStyleSheet("wadadli/css/wadadli.css");
			$this->addStyleSheet("wadadli/css/jquery.datetimepicker.css");
			$this->addStyleSheet("wadadli/third_party/colorpicker/css/colorpicker.css");
			$this->addStyleSheet("wadadli/third_party/tinyeditor/tinyeditor.css");
			$this->addScript("wadadli/js/headers.js",true,"",true);
			$this->addScript("code.jquery.com/jquery-latest.min.js",false);
			$this->addScript("wadadli/js/jquery.datetimepicker.js");
			$this->addScript("wadadli/third_party/colorpicker/js/colorpicker.js");
			$this->addScript("wadadli/third_party/tinyeditor/tiny.editor.js");
			$this->addScript("wadadli/js/wadadli.js");
		}
		
		public function setFavicon($path, $internal=true){
			if(!is_string($path)){
				fatal_error(UNEXPECTED_ARGUMENT_WARNING,"WadadliPage setFavicon expects string path, ".gettype($path)." given.");
				return;
			}
		
			$urlRoot = ($internal)?$this->urlRoot:"";
			$this->favicon = new WadadliComponent();
			$this->favicon->setSelfClosing();
			$this->favicon->setTag('link');
			$this->favicon->addAttribute('rel','icon');
			$this->favicon->addAttribute('href',"{$urlRoot}{$path}");
		}
		
		public function onLoad($newLoadAction,$loadArguments=array()){
			if(!is_string($newLoadAction)){
				fatal_error(UNEXPECTED_ARGUMENT_WARNING,"WadadliPage onLoad expects string action, ".gettype($newLoadAction)." given.");
				return;
			}
			
			if(!is_array($loadArguments)){
				$loadArguments = array($loadArguments);
			}
			
			$this->loadAction = $newLoadAction;
			$this->loadArguments = $loadArguments;
		}
		
		public function addMeta($name, $value){
			$metaTag = new WadadliComponent();
			$metaTag->setSelfClosing();
			$metaTag->setTag('meta');
			$metaTag->addAttribute('name',$name);
			$metaTag->addAttribute('content',$value);
			$this->meta[$name] = $metaTag;
		}
		
		public function addCustomMeta($attributes){
			$metaTag = new WadadliComponent();
			$metaTag->setSelfClosing();
			$metaTag->setTag('meta');
			
			foreach($attributes as $name => $value){
				$metaTag->addAttribute($name,$value);
			}
			
			$this->meta[implode("-",$attributes)] = $metaTag;
		}
		
		public function clearMeta(){
			$this->meta = array();
		}
		
		public function addStylesheet($path, $internal=true){
			if(!is_string($path)){
				fatal_error(UNEXPECTED_ARGUMENT_WARNING,"WadadliPage addStylesheet expects string path, ".gettype($path)." given.");
				return;
			}
			
			$urlRoot = ($internal)?$this->urlRoot:"";
			$stylesheet = new WadadliComponent();
			$stylesheet->setSelfClosing();
			$stylesheet->setTag('link');
			$stylesheet->addAttribute('rel','stylesheet');
			$stylesheet->addAttribute('type','text/css');
			$stylesheet->addAttribute('href',"{$urlRoot}{$path}");
			
			$this->stylesheets[] = $stylesheet;
		}
		
		public function clearStylesheets(){
			$this->stylesheets = array();
		}
		
		public function addScript($path, $internal=true, $inlineCode="",$header=false){
			if($header){
				$this->headerScripts[] = new WadadliScriptComponent($path,"text/javascript",$internal,$inlineCode);
			}else{
				$this->footerScripts[] = new WadadliScriptComponent($path,"text/javascript",$internal,$inlineCode);
			}
		}
		
		public function clearScripts(){
			$this->scripts = array();
		}
		
		public function render(){
			$renderedPage = "<html>";
			$renderedPage .= "<head>";
			$renderedPage .= "<title>{$this->getTitle()}</title>";
			
			if($this->favicon){
				$renderedPage .= $this->favicon->render(false);
			}
			
			foreach(array_merge($this->meta,$this->stylesheets) as $headerElement){
				$renderedPage .= $headerElement->render(false);
			}
			
			foreach($this->headerScripts as $headerElement){
				$renderedPage .= $headerElement->render(false);
			}
			
			$renderedPage .= "</head>";
			
			//Manage on load actions
			$viewZones = $this->getView()->getZones();
			$zoneLoadActions = array();
			foreach($viewZones as $viewZone){
				if($this->getView()->getZoneLoadAction($viewZone)){
					$zoneLoadActions[$viewZone] = $this->getView()->getZoneLoadAction($viewZone);
				}
			}
			
			$loadActions = "";
			if($this->loadAction){
				$loadActions = "{$this->loadAction}(\"".implode("\",\"",$this->loadArguments)."\",this);";
			}
			
			if($zoneLoadActions){
				foreach($zoneLoadActions as $zone => $zoneAction){
					$zoneAction = ActionManager::getAction($zoneAction);
					if(!$zoneAction){
						continue;
					}
					
					$zoneActionUrl = $this->urlRoot."/".$zoneAction->getPath();
					$loadActions .= "loadView(\"$zoneActionUrl\",\"$zone\",false,";
					foreach($this->getView()->getZoneLoadActionArguments($zone) as $zoneActionArgument){
						$loadActions .= "\"$zoneActionArgument\",";
					}
					$loadActions .= "this);";
				}
			}
			
			$onload = "";
			if($loadActions){
				$onload = " onload='$loadActions'";
			}
			$renderedPage .= "<body $onload>";
			
			if($this->getView()){
				$renderedPage .= $this->getView()->render(false);
			}
			
			foreach($this->footerScripts as $footerElement){
				$renderedPage .= $footerElement->render(false);
			}
			
			$renderedPage .= "</body>";
			$renderedPage .= "</html>";
			
			echo "<!DOCTYPE html>\n".str_replace(Calligrapher::$visualNewline,"&#10", Calligrapher::formatMarkup($renderedPage));
		}
	}
?>