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

var lastQuery = '';

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

        // If the current query equals the one we just executed,
        // do nothing.
        if($form.serialize() === lastQuery) {
            return;
        }
        lastQuery = $form.serialize();

        //show some response on the button
        $('button[type="submit"]', $form).each(function () {
            $btn = $(this);
            $btn.prop('type', 'button');
            $btn.prop('orig_label', $btn.text());
            $btn.text('Sending ...');
			$('#loading-container').show();
        });

        const searchTerms = $('#icon_prefix2').val();

        $.ajax({
            type: "GET",
            crossDomain: true,
            url: "/search",
            data: $form.serialize(),
            success: after_form_submitted,
            error: function(XMLHttpRequest, textStatus, errorThrown) {
                alert("Status: " + textStatus + "\nError: " + errorThrown);
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

    const deduplicatedPages = [];
    data.pages.forEach(page => {
        for (let i = 0; i < deduplicatedPages.length; i++) {
            if(deduplicatedPages[i].title === page.title)
                return;
        }
        deduplicatedPages.push(page);
    });

    $.each(deduplicatedPages, function (key, val) {
        if (key >= 30) {
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
    
	$('#loading-container').hide();

    // Show search results
    $('#search-results').show();
    $('#search-results').empty();
    $('#search-results').append(toInsert);
}
