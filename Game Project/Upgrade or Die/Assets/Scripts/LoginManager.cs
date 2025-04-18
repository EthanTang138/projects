using UnityEngine;
using UnityEngine.UI;
using UnityEngine.Networking;
using UnityEngine.SceneManagement;
using System.Collections;

public class LoginManager : MonoBehaviour
{
    public InputField usernameInput;
    public InputField passwordInput;
    public Text feedbackText;

    private string baseUrl = "http://localhost:3000/api/users";

    public void OnRegisterPressed()
    {
        StartCoroutine(Register(usernameInput.text, passwordInput.text));
    }

    public void OnLoginPressed()
    {
        StartCoroutine(Login(usernameInput.text, passwordInput.text));
    }

    IEnumerator Register(string username, string password)
    {
        string url = baseUrl + "/register";
        string json = JsonUtility.ToJson(new AuthData(username, password));

        UnityWebRequest request = new UnityWebRequest(url, "POST");
        byte[] body = System.Text.Encoding.UTF8.GetBytes(json);
        request.uploadHandler = new UploadHandlerRaw(body);
        request.downloadHandler = new DownloadHandlerBuffer();
        request.SetRequestHeader("Content-Type", "application/json");

        yield return request.SendWebRequest();

        if (request.result == UnityWebRequest.Result.Success && request.responseCode == 200)
        {
            feedbackText.text = "✅ Registered!";
        }
        else
        {
            feedbackText.text = $"❌ Register failed: {request.responseCode}";
        }
    }

    IEnumerator Login(string username, string password)
    {
        string url = baseUrl + "/login";
        string json = JsonUtility.ToJson(new AuthData(username, password));

        UnityWebRequest request = new UnityWebRequest(url, "POST");
        byte[] body = System.Text.Encoding.UTF8.GetBytes(json);
        request.uploadHandler = new UploadHandlerRaw(body);
        request.downloadHandler = new DownloadHandlerBuffer();
        request.SetRequestHeader("Content-Type", "application/json");

        yield return request.SendWebRequest();

        if (request.result == UnityWebRequest.Result.Success && request.responseCode == 200)
        {
            feedbackText.text = "✅ Login successful!";
            SceneManager.LoadScene("WaitingRoom");
        }
        else
        {
            feedbackText.text = $"❌ Login failed: {request.responseCode}";
        }
    }

    [System.Serializable]
    public class AuthData
    {
        public string username;
        public string password;

        public AuthData(string u, string p)
        {
            username = u;
            password = p;
        }
    }
}
