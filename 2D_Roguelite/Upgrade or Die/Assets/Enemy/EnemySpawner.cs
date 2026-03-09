using UnityEngine;
using Mirror;

public class EnemySpawner : NetworkBehaviour
{
    public GameObject enemyPrefab;
    public Transform[] spawnPoints;
    public int numberOfEnemies = 3;

    public override void OnStartServer()
    {
        for (int i = 0; i < numberOfEnemies; i++)
        {
            Transform spawnPoint = spawnPoints[Random.Range(0, spawnPoints.Length)];
            GameObject enemy = Instantiate(enemyPrefab, spawnPoint.position, Quaternion.identity);
            NetworkServer.Spawn(enemy); // Ensure it's spawned for all clients
        }
    }
}