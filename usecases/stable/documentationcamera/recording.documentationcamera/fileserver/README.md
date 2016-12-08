This folder provides the instructions for sharing video recordings with other machines connected in the same local network. Suppose that there are video recordings in ~/recordings of the local host. Run the fileserver micro-service:

    $ docker-compose up --build
    
Then point a web-browser to "http://localhost" or to the machine where recordings have been conducted. A list of the recording files will be available for download in the web-browser.

To stop the fileserver and remove the stopped Docker container, run

    $ docker-compose stop

    $ docker-compose rm

