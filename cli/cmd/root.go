package cmd

import (
  "os"
  "github.com/spf13/cobra"
)

var rootCmd = &cobra.Command{
  Use:   "wge-cli",
  Short: "Manage games powered by WGE",
  Long:  `wge-cli is a CLI tool that initializes a customized template to start making a game out of it based upon the WGE engine/framework.`,
}

func Execute() {
  err := rootCmd.Execute()
  if err != nil {
    os.Exit(1)
  }
}

func init() {
  rootCmd.AddCommand(newCmd)
}
