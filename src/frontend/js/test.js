// Inserting results like below:

// <div className="result collection-item">
//     <a href="https://zackLight.com" target="_blank">Page Title</a>
//     <div className="snippet">
//         <div className="col s12"><p>
//             Lorem ipsum dolor sit amet, consectetur adipisicing elit. Aliquid consequatur cumque eaque
//             iste natus nulla officia officiis optio sequi similique temporibus, voluptas voluptate. Beatae culpa dicta
//             eius quo rem similique!
//         </p></div>
//     </div>
// </div>
//
// <div className="result collection-item">
//     <a href="https://zackLight.com" target="_blank">Page Title</a>
//     <div className="snippet">
//         <div className="col s12"><p>
//             Lorem ipsum dolor sit amet, consectetur adipisicing elit. Aliquid consequatur cumque eaque
//             iste natus nulla officia officiis optio sequi similique temporibus, voluptas voluptate. Beatae culpa dicta
//             eius quo rem similique!
//         </p></div>
//     </div>
// </div>

function insertResults(data) {
    let toInsert = ``

    $.each(data.pages, function (key, val) {
        if (key >= 10) {
            return
        }
        toInsert += `
            <div class="result collection-item">
                <a href="https://www.google.com/search?q=${val.name}" target="_blank">${val.name}</a>
                <div class="snippet">
                    <div class="col s12"><p>
                        ${val.snippet[0]}
                    </p></div>
                </div>
            </div>
        `
    });

    $('#search-results').append(toInsert);
}
