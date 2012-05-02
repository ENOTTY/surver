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

function put(data, callback) {
	$.ajax('/', {
		type: 'POST',
		data: JSON.stringify(data),
		contentType: 'text/json',
		success: function() { if ( callback ) callback(true); },
		error: function() { if ( callback ) callback(false); },
	});
}

function getCheckedValue(elems, name) {
	var i;
	for (i = 0; i < elems.length; ++i) {
		if (elems[i].name == name)
			return elems[i].value;
	}
	return null;
}

$(function() {
	$('#survey').submit(function(e) {
		e.preventDefault();

		var checked = $('input:checked');

		var entry = new Object();
		entry.course = $('#id-course').val();
		entry.content = getCheckedValue(checked, 'cont');
		entry.labs = getCheckedValue(checked, 'labs');
		entry.organization = getCheckedValue(checked, 'org');
		entry.courseComment = $('#id-course-comment').val();
		entry.name = $('#id-poc-name').val();
		entry.sid = $('#id-poc-sid').val();

		entry.instructors = new Array();
		for (i=0; i < 10; ++i) { //more than 10 instructors?
			var nameElem = $('[name=instname-'+i+']');
			if (nameElem.length != 1)
				break;
			var instructor = new Object();
			entry.instructors[i] = instructor;
			instructor.name = nameElem.val();
			instructor.knowledge = getCheckedValue(checked, "know"+i);
			instructor.prep = getCheckedValue(checked, "prep"+i);
			instructor.comm = getCheckedValue(checked, "comm"+i);
		}

		put(entry, function(success) {
			if (success) {
				alert('success!');
			} else {
				alert('error!');
			}
		});
	});
});

