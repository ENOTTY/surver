var instructor_cnt = new Number(0);

function serializeForm()
{
	var old_values = new Array();
	var itr, obj;
	
	for (var i = 0; i < document.survey.elements.length; i++) {
        itr = document.survey.elements[i];

        // Only backup elements with IDs
        if (itr.id) {

            // Allocate and copy the form element data
            obj = new Object();

            if (itr.id) {
                obj.id = itr.id;
            }

            if (itr.value) {
                obj.value = itr.value;
            }

            if (itr.checked) {
                obj.checked = itr.checked;
            }

            old_values.push(obj);
        }
    }

	return old_values;
}

function unserializeForm(old_values)
{
	var itr;
    // Apply the new form code
    for (var i = 0; i < old_values.length; i++) {
        itr = document.getElementById(old_values[i].id);
        if (itr) {
            if (old_values[i].checked) {
                itr.checked = old_values[i].checked;
            }
            if (old_values[i].value) {
                itr.value = old_values[i].value;
            }
        }
    }
}

function addInstructor()
{
    var current_vals, current_checked, template, old_values;

		// This is a hardcoded bound
    if (instructor_cnt >= 99)
        return;

		old_values = serializeForm();	

    template = document.getElementById("instructor-review-template").innerHTML;
    template = template.replace(/NUM/g, instructor_cnt);

    // Append the new form code
    document.getElementById("instructor-reviews").innerHTML += template;

		unserializeForm(old_values);

    // Increment the counter id
    instructor_cnt += 1;
}

function postJson()
{
	document.survey.elements['json'].value = JSON.stringify(serializeForm());
}

