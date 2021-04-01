$(function () {
    function after_form_submitted(data) {
        if (data.result == 'success') {
            $('form#search-form').hide();
            $('#success_message').show();
            $('#error_message').hide();
        } else {
            $('#error_message').append('<ul></ul>');

            jQuery.each(data.errors, function (key, val) {
                $('#error_message ul').append('<li>' + key + ':' + val + '</li>');
            });
            $('#success_message').hide();
            $('#error_message').show();

            //reverse the response on the button
            $('button[type="button"]', $form).each(function () {
                $btn = $(this);
                label = $btn.prop('orig_label');
                if (label) {
                    $btn.prop('type', 'submit');
                    $btn.text(label);
                    $btn.prop('orig_label', '');
                }
            });

        }//else
    }

    $('#search-form').submit(function (e) {
        e.preventDefault();

        $form = $(this);
        //show some response on the button
        $('button[type="submit"]', $form).each(function () {
            $btn = $(this);
            $btn.prop('type', 'button');
            $btn.prop('orig_label', $btn.text());
            $btn.text('Sending ...');
        });

        console.log("query: ", $form.serialize())
        insertResults()

        // TODO make requests to all servers

        //         $.ajax({
        //     type: "POST",
        //     url: 'handler.php',
        //     data: $form.serialize(),
        //     success: after_form_submitted,
        //     dataType: 'json'
        // });
    });

    let search_bar = document.querySelector("#search-bar")
    search_bar.addEventListener("keydown", event => {
        if (event.key !== "Enter") return;
        document.querySelector("#search-btn").click();
        event.preventDefault();
    });
});