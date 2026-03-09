using System.Collections;
using UnityEngine;
using UnityEngine.UI;
using UnityEngine.Networking;
using UnityEngine.SceneManagement; 
using TMPro;

public class Login: MonoBehaviour
{
    public TMP_InputField usernameInput;
    public TMP_InputField passwordInput;
    public TMP_Text statusText;
    private string serverUrl = "http://192.168.1.100:8080"; // Replace with your Spring Boot server IP

    public void AttemptLogin()
    {
        StartCoroutine(LoginRequest(usernameInput.text, passwordInput.text));
    }

    IEnumerator LoginRequest(string username, string password)
    {
        string url = serverUrl + "/auth/login?username=" + username + "&password=" + password;
        UnityWebRequest request = UnityWebRequest.PostWwwForm(url, "");

        yield return request.SendWebRequest();

        if (request.result == UnityWebRequest.Result.Success)
        {
            statusText.text = "Login Successful!";
            PlayerPrefs.SetString("username", username); // Store session
            SceneManager.LoadScene("Game Environment"); // Load main game scene
        }
        else
        {
            statusText.text = "Login Failed: " + request.downloadHandler.text;
        }
    }
}
