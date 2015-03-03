<?php	
	class Belongs extends Annotation{
	}
	
	abstract class WadadliAnnotation extends Annotation{
		public $required;
	}
	
	class Object extends WadadliAnnotation{
		public $class;
	}
	
	class Date extends WadadliAnnotation {}
	
	abstract class DefaultedAnnotation extends WadadliAnnotation{
		public $default;
	}
	
	class Boolean extends DefaultedAnnotation {}
	
	abstract class ValuedAnnotation extends DefaultedAnnotation{
		public $values;
	}
	
	abstract class RangedAnnotation extends ValuedAnnotation{
		public $min;
		public $max;
	}
	
	class String extends RangedAnnotation {}
	class URL extends String {}
	class Email extends String {}
	class Phone extends String {}
	class Password extends String {}
	class File extends String {}
	class Image extends File {
		public $height;
		public $width;
	}
	class Text extends String {}
	
	class Integer extends RangedAnnotation {}
	class Float extends RangedAnnotation {}
	class Monetary extends RangedAnnotation {}
	
	class Collection extends RangedAnnotation{
		public $type;
	}
?>