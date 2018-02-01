# WebPro
To load a webpage, a web browser first downloads the base HTML file of the page in order to discover the list of objects referenced in the page. This process takes roughly one round-trip time and constitutes a significant portion of the web browsing delay on mobile devices as wireless networks suffer from longer transmission and access delays compared to wired networks. In this work, we propose a solution for eliminating this initial delay, which is transparent to end systems, does not require modifying HTTP, and is well suited for web browsing on mobile devices. Our solution, called WebPro, is built upon two cooperating proxies, local proxy and remote proxy.

Local proxy resides in the mobile device, where the mobile browser is configured to use the local proxy for all of its network operations. After receiving the request to load a page, local proxy will forward the requested URL to the remote proxy, which is deployed in the backbone of the cellular network.

Remote proxy is equipped with a profiler that builds an up-to-date database of resource lists for the websites visited frequently by network users. Given that the remote proxy resides in the wired part of the network, it can afford to pro-actively build and refresh the resource list database periodically. When a request for a webpage comes to the remote proxy, it simultaneously fetches the base HTML and all referenced objects required to render the webpage using the corresponding resource list stored in the local database. After fetching all the objects, the remote proxy packs them in a bundle and sends the bundle to local proxy. After receiving the bundle at the mobile device, local proxy will serve all the browser's requests from objects in the bundle.
