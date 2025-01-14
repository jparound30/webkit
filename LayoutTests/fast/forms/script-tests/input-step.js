description('Test the behavior of step attribute and step DOM property.');

var input = document.createElement('input');
input.type = 'number';

// No step attribute.
shouldBe('input.step', '""');

// Set an invalid step value.
input.step = 'foo';
shouldBe('input.getAttribute("step")', '"foo"');
input.setAttribute('step', 'bar');
shouldBe('input.step', '"bar"');

// Null.
input.step = null;
shouldBe('input.step', '""');
shouldBe('input.getAttribute("step")', 'null');
input.setAttribute('step', null);
shouldBe('input.step', '"null"');

// Undefined.
input.step = undefined;
shouldBe('input.step', '"undefined"');
shouldBe('input.getAttribute("step")', '"undefined"');
input.setAttribute('step', undefined);
shouldBe('input.step', '"undefined"');

// Non-string.
input.step = 256;
shouldBe('input.step', '"256"');
shouldBe('input.getAttribute("step")', '"256"');
input.setAttribute('step', 256);
shouldBe('input.step', '"256"');
