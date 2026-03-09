using UnityEngine;
using Mirror;
using TMPro;

public class PlayerStats : NetworkBehaviour
{
    [SyncVar(hook = nameof(OnKillsChanged))]
    public int kills;

    [SyncVar(hook = nameof(OnDeathsChanged))]
    public int deaths;

    public TextMeshProUGUI killsText;
    public TextMeshProUGUI deathsText;

    public override void OnStartLocalPlayer()
    {
        // Find and assign the text elements from PlayerUI
        killsText = GameObject.Find("KillsText")?.GetComponent<TextMeshProUGUI>();
        deathsText = GameObject.Find("DeathsText")?.GetComponent<TextMeshProUGUI>();

        // Initialize UI
        UpdateKillsText();
        UpdateDeathsText();
    }

    public void AddKill()
    {
        if (isServer)
            kills++;
    }

    public void AddDeath()
    {
        if (isServer)
            deaths++;
    }

    void OnKillsChanged(int _, int newKills)
    {
        UpdateKillsText();
    }

    void OnDeathsChanged(int _, int newDeaths)
    {
        UpdateDeathsText();
    }

    void UpdateKillsText()
    {
        if (isLocalPlayer && killsText != null)
            killsText.text = $"Kills: {kills}";
    }

    void UpdateDeathsText()
    {
        if (isLocalPlayer && deathsText != null)
            deathsText.text = $"Deaths: {deaths}";
    }
}
