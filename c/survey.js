var count = new Number(0);

function addInstructor()
{
    var current_vals, current_checked, template;

    // Get all the current form values
    var old_values = new Array();
    var itr, obj;

    if (count >= 99)
        return;

    for (var i = 0; i < document.forms[0].elements.length; i++) {
        itr = document.forms[0].elements[i];

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

    template = document.getElementById("instructor-review-template").innerHTML;
    template = template.replace(/NUM/g, count);

    // Append the new form code
    document.getElementById("instructor-reviews").innerHTML += template;

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

    // Increment the counter id
    count += 1;
}