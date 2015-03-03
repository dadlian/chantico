<?php
	class WadadliForm extends WadadliComponent{
		/** @String */
		protected $title;
		
		/** @String(default='') */
		protected $submitLabel;
		
		/** @String */
		protected $submitAction;
		
		/** @String(default='') */
		protected $cancelLabel;
		
		/** @String */
		protected $cancelAction;
		
		/** @Collection(type=@Collection(type=@Object(class='WadadliComponent'))) */
		protected $inputs;
		
		/** @Boolean(default=false) */
		protected $dynamic;
		
		/** @String(default="") */
		protected $target;
		
		private $submitArguments;
		private $cancelArguments;
		private $asyncCallback;
		private $asyncCallbackArg;
		
		public function __construct(){
			call_user_func_array(array('parent','__construct'),func_get_args());
			$action = ActionManager::getAction($this->getSubmitAction());
			
			if($action){
				$actionUrl = SettingsManager::getSetting('Install','Url Root')."/{$action->getPath()}";
				$actionUrl = (preg_match("/^http/",$actionUrl)?"":"http://").$actionUrl;
				$this->addAttribute('action',$actionUrl);
			}else{
				$this->addAttribute('action',"#");
			}
			
			$this->setTag('form');
			$this->addAttribute('method','post');
			$this->addAttribute('enctype','multipart/form-data');
			
			$this->submitArguments = array();
			$this->cancelArguments = array();
			$this->asyncCallback = "";
			$this->asyncCallbackArg = "";
		}
		
		public function addInput($component, $section="none"){
			if(!$section){
				$section = "none";
			}
		
			$sectionComponents = $this->viewFromInputs($section);
			if(!$sectionComponents){
				$sectionComponents = array();
			}
			$sectionComponents[] = $component;
			
			$this->insertToInputs($section,$sectionComponents);
		}
		
		public function addSubmitArgument($argument){
			if(!is_string($argument)){
				fatal_error(UNEXPECTED_ARGUMENT_WARNING,"WadadliForm expects string submit arguments, ".gettype($argument)." given");
				return;
			}
			
			$this->addAttribute('action',preg_replace('/@/',$argument,$this->getAttribute('action'),1));
			
			$this->submitArguments[] = $argument;
		}
		
		public function addCancelArgument($argument){
			if(!is_string($argument)){
				fatal_error(UNEXPECTED_ARGUMENT_WARNING,"WadadliForm expects string cancel arguments, ".gettype($argument)." given");
				return;
			}
			
			$this->cancelArguments[] = $argument;
		}
		
		public function asynchronise($target,$callback="",$callbackArg=""){
			$this->setDynamic(true);
			$this->setTarget($target);
			
			$arguments = array($this->getAttribute('action'),$target,$callback,$callbackArg,"#{$this->getComponentId()}");
			$arguments = array_merge($arguments,$this->submitArguments);
			$this->onSubmit('return submitAsyncForm',$arguments);
			
			$this->asyncCallback = $callback;
			$this->asyncCallbackArg = $callbackArg;
		}
		
		protected function renderComponent(){
			$renderedComponent = "";
			
			if($this->getTitle()){
				$heading = new WadadliHeading("{$this->getComponentId()}Heading",$this->getTitle(),1);
				$renderedComponent .= $heading->render(false);
			}
			
			$messageBox = new WadadliSystemMessageBox();
			$renderedComponent .= $messageBox->render(false);
			
			$sectionlessInputs = ($this->viewFromInputs('none'))?$this->viewFromInputs('none'):array();
			if($sectionlessInputs){
				$section = new WadadliContainer("{$this->getComponentId()}None");
				$section->addClass('formSection');
				foreach($sectionlessInputs as $formInput){
					$section->addContent($formInput);
				}
				$renderedComponent .= $section->render(false);
			}
			
			foreach($this->getInputs() as $sectionName => $inputs){
				if($sectionName !== 'none'){
					$section = new WadadliContainer("{$sectionName}FormSection");
					$section->addClass('formSection');
					$section->addContent(new WadadliHeading(decapitalise("{$sectionName}Heading"),$sectionName,3));
				
					foreach($inputs as $formInput){
						$section->addContent($formInput);
					}
					
					$renderedComponent .= $section->render(false);
				}
			}
			
			$buttonPanel = new WadadliContainer("{$this->getComponentId()}ButtonPanel");
			$buttonPanel->addClass('buttonPanel');
			
			if($this->getSubmitLabel()){
				$buttonPanel->addContent(new WadadliButton($this->getSubmitLabel(),"",false,true));
			}
			
			if($this->getCancelAction()){
				$cancelButton = new WadadliButton($this->getCancelLabel());
				
				$action = ActionManager::getAction($this->getCancelAction());
				$actionUrl = SettingsManager::getSetting('Install','Url Root').($action?$action->getPath():"");
				$actionUrl = preg_replace("/\/+/","/",$actionUrl);
				
				foreach($this->cancelArguments as $argument){
					$actionUrl = preg_replace("/@/",$argument,$actionUrl,1);
				}
				
				if($this->isDynamic()){
					$cancelButton->loadView('click',$this->getCancelAction(),$this->getTarget(),$this->cancelArguments,false,
									$this->asyncCallback,$this->asyncCallbackArg);
				}else{
					$cancelButton->onClick('redirect',array($actionUrl));
				}
				
				$buttonPanel->addContent($cancelButton);
			}
			
			$renderedComponent .= $buttonPanel->render(false);
			
			return $renderedComponent;
		}
	}
?>