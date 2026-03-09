using UnityEngine;
using Mirror;
using UnityEngine.UI;
using TMPro;

public enum SkillType { None, Fireball, Dash, Shield }
public class Experience : NetworkBehaviour
{
    [Header("XP & Level System")]
    [SyncVar(hook = nameof(OnXPChanged))]
    private int xp = 0;

    [SyncVar(hook = nameof(OnLevelChanged))]
    private int level = 1; // Level should sync properly

    public Slider xpBar;
    public TMP_Text levelText;
    private TMP_Text abilityText;
    private int xpToLevelUp = 100;
    private int xpleft = 0;

    public GameObject playerUIPrefab;
    private GameObject playerUIInstance;
    private GameObject inventoryPanel;
    public GameObject skillSelectionPanel;
    public Button skillButton1; 
    public Button skillButton2; 
    public Button skillButton3;

    [SyncVar] public bool unlockedFireball = false;
    [SyncVar] public bool unlockedDash = false;
    [SyncVar] public bool unlockedShield = false;

    [SyncVar]
    public SkillType currentSkill = SkillType.None;
    public override void OnStartLocalPlayer()
    {
        base.OnStartLocalPlayer();

        if (!isLocalPlayer) return; // Only create UI for the local player

        // Prevent multiple UI instances
        if (GameObject.Find("PlayerUI(Clone)") != null)
        {
            Debug.LogWarning("Duplicate PlayerUI detected! Skip creating extra instance.");
            return;
        }

        playerUIInstance = Instantiate(playerUIPrefab);
        playerUIInstance.transform.SetParent(GameObject.Find("Canvas").transform, false);
        inventoryPanel = playerUIInstance.transform.Find("InventoryPanel").gameObject;
        xpBar = playerUIInstance.transform.Find("XPbar").GetComponent<Slider>();
        levelText = playerUIInstance.transform.Find("Level").GetComponent<TMP_Text>();
        abilityText = playerUIInstance.transform.Find("InventoryPanel/AbilityText").GetComponent<TMP_Text>();
        inventoryPanel.SetActive(false);

        if (skillSelectionPanel != null)
            skillSelectionPanel.SetActive(false);

        // 给按钮添加事件（也可以直接在 Inspector 中设置 OnClick）
        if (skillButton1 != null)
            skillButton1.onClick.AddListener(() => OnSelectSkill(SkillType.Fireball));
        if (skillButton2 != null)
            skillButton2.onClick.AddListener(() => OnSelectSkill(SkillType.Dash));
        if (skillButton3 != null)
            skillButton3.onClick.AddListener(() => OnSelectSkill(SkillType.Shield));
        UpdateXPUI();
    }

    [Command]
    /* Old Version
    public void CmdAddXP(int amount)
    {
        xp += amount;
        if (xp >= xpToLevelUp)
        {
            xpleft = xp - xpToLevelUp;
            xp = 0;
            xp = xp + xpleft;
            level++;
            RpcShowLevelUpUI();  
        }
    }*/
    public void CmdAddXP(int amount)
    {
        if (!isServer) return; // Only the server modifies XP

        xp += amount;
        Debug.Log($"XP gained: {amount}. Total XP: {xp}/{xpToLevelUp}");

        if (xp >= xpToLevelUp)
        {
            LevelUp();
        }
    }

    [Server]
    public void AddXP(int amount)
    {
        xp += amount;
        Debug.Log($"XP gained: {amount}. Total XP: {xp}/{xpToLevelUp}");

        if (xp >= xpToLevelUp)
        {
            LevelUp();
        }
    }

    [Server]
    private void LevelUp()
    {
        xpleft = xp - xpToLevelUp;
        xp = xpleft; // Keep the remaining XP after level-up
        level++;
        xpToLevelUp += 50; // Increase XP required for next level

        Debug.Log($"Player leveled up! New Level: {level}");

        RpcShowLevelUpUI();
    }

    /* Old Version
    void OnXPChanged(int oldXP, int newXP)
    {
        if (xpBar != null)
            xpBar.value = (float)newXP / xpToLevelUp;
    }
    void OnLevelChanged(int oldLevel, int newLevel)
    {
        if (levelText != null)
            levelText.text = "Level: " + newLevel;
    }*/
    void OnXPChanged(int oldXP, int newXP)
    {
        if (xpBar != null)
            xpBar.value = (float)newXP / xpToLevelUp;
    }

    void OnLevelChanged(int oldLevel, int newLevel)
    {
        if (levelText != null)
            levelText.text = $"Level: {newLevel}";
    }

    [ClientRpc]
    /* Old Version
    void RpcShowLevelUpUI()
    {
        if (isLocalPlayer)
        {
            Debug.Log("Level Up! Choose a new ability!");
            // Here, you can show a UI panel for ability selection
        }
    }
    void UpdateXPUI()
    {
        if (xpBar != null)
            xpBar.value = (float)xp / xpToLevelUp;

        if (levelText != null)
            levelText.text = $"Level: {level}"; // Works with TextMeshPro
    }*/
    void RpcShowLevelUpUI()
    {
        if (isLocalPlayer)
        {
            Debug.Log("Level Up! Choose a new ability!");
            if (skillSelectionPanel != null)
                skillSelectionPanel.SetActive(true);
            // Show ability selection UI if needed
        }
    }
    void UpdateXPUI()
    {
        if (xpBar != null)
            xpBar.value = (float)xp / xpToLevelUp;

        if (levelText != null)
            levelText.text = $"Level: {level}";
    }

    public void OnSelectSkill(SkillType skill)
    {
        if (!isLocalPlayer) return;
        switch (skill)
        {
            case SkillType.Fireball:
                unlockedFireball = true;
                Debug.Log("Fireball unlocked!");
                break;
            case SkillType.Dash:
                unlockedDash = true;
                Debug.Log("Dash unlocked!");
                break;
            case SkillType.Shield:
                unlockedShield = true;
                Debug.Log("Shield unlocked!");
                break;
        }
        // 关闭技能选择界面
        if (skillSelectionPanel != null)
            skillSelectionPanel.SetActive(false);
    }
}

