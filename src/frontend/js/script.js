$(function () {
    function after_form_submitted(data) {
        insertResults(data)

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

        $.ajax({
            type: "GET",
            crossDomain: true,
            url: 'http://localhost:8080/index.html',
            data: $form.serialize(),
            success: after_form_submitted,
            error: function(XMLHttpRequest, textStatus, errorThrown) {
                alert("Status: " + textStatus); alert("Error: " + errorThrown);
            },
            dataType: 'json'
        });
    });

    let search_bar = document.querySelector("#search-bar")
    search_bar.addEventListener("keydown", event => {
        if (event.key !== "Enter") return;
        document.querySelector("#search-btn").click();
        event.preventDefault();
    });
});