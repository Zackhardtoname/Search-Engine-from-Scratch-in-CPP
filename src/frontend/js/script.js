// Example json response from the C++ server:
//
// {
//         "pages":        [
//                 {
//                         "title": "Title",
//                         "url": "URL",
//                         "snippet": "Snippet",
//                         "rank": "5"
//                 },
//                 {
//                         "title": "Title",
//                         "url": "URL",
//                         "snippet": "Snippet",
//                         "rank": "5"
//                 },
//                 {
//                         "title": "Title",
//                         "url": "URL",
//                         "snippet": "Snippet",
//                         "rank": "5"
//                 }
//         ]
// }


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

        const searchTerms = $('#icon_prefix2').val();

        // Show search results
        $('#search-results').show();

        $.ajax({
            type: "GET",
            crossDomain: true,
            url: "/search",
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


function insertResults(data) {
    let toInsert = ``

    $.each(data.pages, function (key, val) {
        if (key >= 10) {
            return
        }
        toInsert += `
            <div class="result collection-item">
                <a href="${val.url}" target="_blank">${val.title}</a>
                <div class="rank">
                    <div class="col s12"><p>
                        Ranker score: ${val.rank}
                    </p></div>
                </div>
            </div>
        `
    });

    $('#search-results').append(toInsert);
}
