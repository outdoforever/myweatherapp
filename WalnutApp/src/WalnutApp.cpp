#include "Walnut/Application.h"
#include "Walnut/EntryPoint.h"
#include "Walnut/Image.h"

#include <iostream>
#include <string>
#include <cpprest/http_client.h>
#include <cpprest/filestream.h>
using namespace std;
using namespace web;
using namespace web::http; 
using namespace web::http::client;

class ExampleLayer : public Walnut::Layer
{
public:
    virtual void OnAttach() override
    {
        // Initialize the weather data
        city = "";
        temperature = "";
        humidity = "";
        condition = "";
    }

    virtual void OnUIRender() override
    {
        ImGui::Begin("Weather Monitoring System");

        ImGui::Text("Welcome to the Weather Monitoring System!");
        ImGui::TextWrapped("Plan your day with real-time weather updates. Enter the city name below:");
        ImGui::Separator();
        ImGui::Spacing();

        static char cityBuffer[256] = "";
        ImGui::InputTextWithHint("##CityInput", "Type a city name here...", cityBuffer, sizeof(cityBuffer));

        ImGui::Spacing();
        if (ImGui::Button("Get Weather", ImVec2(150, 40)))
        {
            city = string(cityBuffer);
            FetchWeatherData(city);
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        if (!temperature.empty())
        {
            ImGui::Text("Weather Information:");
            ImGui::Separator();
            ImGui::Text("City: %s", city.c_str());
            ImGui::Text("Temperature: %s K", temperature.c_str());
            ImGui::Text("Humidity: %s%%", humidity.c_str());
            ImGui::Text("Condition: %s", condition.c_str());

            ImGui::Spacing();
            ImGui::TextWrapped("Stay safe and have a wonderful day!");
        }
        else
        {
            ImGui::TextWrapped("Enter a city name and press 'Get Weather' to see the forecast.");
        }

        ImGui::End();
    }

private:
    void FetchWeatherData(const string& cityName)
    {
        try
        {
            // Set up the HTTP client and request
            http_client client(U("https://api.openweathermap.org/data/2.5"));
            uri_builder builder(U("/weather"));
            builder.append_query(U("q"), utility::conversions::to_string_t(cityName));
            builder.append_query(U("appid"), U("d1499edaf881207975639c3498d3864b"));

            http_request request(methods::GET);
            request.set_request_uri(builder.to_string());

            // Send the request and process the response
            client.request(request)
                .then([](http_response response) {
                if (response.status_code() != status_codes::OK)
                {
                    throw runtime_error("Failed to get weather data: " + to_string(response.status_code()));
                }
                return response.extract_json();
                    })
                .then([this](web::json::value body) {
                // Extract weather information
                double temp = body[U("main")][U("temp")].as_double();
                double hum = body[U("main")][U("humidity")].as_double();
                string weather = utility::conversions::to_utf8string(body[U("weather")][0][U("main")].as_string());

                // Update the UI variables
                temperature = to_string(temp);
                humidity = to_string(hum);
                condition = weather;
                    })
                .wait();
        }
        catch (const exception& ex)
        {
            cerr << "Error: " << ex.what() << endl;
            temperature = "Error";
            humidity = "Error";
            condition = "Error";
        }
    }

    string city;
    string temperature;
    string humidity;
    string condition;
};

Walnut::Application* Walnut::CreateApplication(int argc, char** argv)
{
    Walnut::ApplicationSpecification spec;
    spec.Name = "Weather Monitoring System";

    Walnut::Application* app = new Walnut::Application(spec);
    app->PushLayer<ExampleLayer>();
    app->SetMenubarCallback([app]() {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Exit"))
            {
                app->Close();
            }
            ImGui::EndMenu();
        }
        });
    return app;
}
